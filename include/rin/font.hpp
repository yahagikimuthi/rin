#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <span>
#include <unordered_map>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "rin/detail/stb_truetype.h"
#pragma GCC diagnostic pop

#include "rin/detail/default_font_data.hpp"
#include "rin/error.hpp"
#include "rin/extent.hpp"
#include "rin/texture.hpp"
#include "rin/types.hpp"
#include "rin/vec2.hpp"

/**
 * @file font.hpp
 * @brief フォントファイルの読み込み、アトラス生成、グリフ情報の管理を行うモジュール
 */

namespace rin {

/**
 * @struct glyph
 * @brief グリフ座標
 */
struct glyph final {
    /// UV長方形
    uv_rectangle uv_rect{};

    /// 文字サイズ
    extent size{};

    /// 基準位置から文字を描画し始まる左隅までの距離
    vec2   bearing{};

    /// 送り量
    f32    advance{};
};

/// デフォルトのフォントサイズ（ピクセル単位）
inline constexpr auto default_font_size = 48.f;

/**
 * @class font
 * @brief
 * フォントアトラス・テクスチャおよび各コードポイント（ASCII 32..127）に対応するグリフ情報を保持するクラス
 *
 * @details stb_truetype を用いてフォントバイナリからテクスチャアトラスを生成・管理します。
 * 直接のコンストラクタ呼び出しは行わず、静的ファクトリ関数 `try_make` またはフリー関数
 * `try_make_font` を使用して生成します。
 */
class font final {
  public:
    /**
     * @brief ファイルパスを指定してフォントインスタンスの生成を試みます。
     *
     * @param path フォントファイル（.ttf 等）のパス
     * @param atlas_width 生成するテクスチャアトラスの幅（ピクセル）
     * @param atlas_height 生成するテクスチャアトラスの高さ（ピクセル）
     * @return std::expected<font, error> 成功した場合は font インスタンス、失敗した場合はエラー情報
     */
    [[nodiscard]] static auto try_make(
        const std::filesystem::path& path,
        const u32                    atlas_width  = 1024,
        const u32                    atlas_height = 1024
    ) noexcept -> std::expected<font, error> {
        auto file = std::ifstream{path, std::ios::binary | std::ios::ate};
        if (not file.is_open()) return make_error(logic_error, "Failed to open font file.");

        const auto               file_size   = file.tellg();
        static thread_local auto font_buffer = std::vector<u8>{};
        font_buffer.resize(static_cast<std::size_t>(file_size));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(font_buffer.data()), file_size);  // NOLINT

        return try_make(font_buffer, atlas_width, atlas_height);
    }

    /**
     * @brief メモリ上のバイナリデータからフォントインスタンスの生成を試みます。
     *
     * @param buffer フォントファイルのバイナリデータ（デフォルトは標準埋め込み Proggy Clean
     * バイナリ）
     * @param atlas_width 生成するテクスチャアトラスの幅（ピクセル）
     * @param atlas_height 生成するテクスチャアトラスの高さ（ピクセル）
     * @return std::expected<font, error> 成功した場合は font インスタンス、失敗した場合はエラー情報
     */
    [[nodiscard]] static auto try_make(
        const std::span<const u8> buffer       = detail::default_font_binary,
        const u32                 atlas_width  = 1024,
        const u32                 atlas_height = 1024
    ) noexcept -> std::expected<font, error> {
        if (buffer.empty()) return make_error(logic_error, "Buffer size is expected positive.");
        auto atlas_pixels = std::vector<u8>(static_cast<std::size_t>(atlas_width * atlas_height));
        auto baked_chars  = std::vector<stbtt_bakedchar>(96);  // ASCII 32..127 (96文字)

        const auto res = stbtt_BakeFontBitmap(
            buffer.data(),
            0,
            default_font_size,
            atlas_pixels.data(),
            static_cast<i32>(atlas_width),
            static_cast<i32>(atlas_height),
            32,
            96,
            baked_chars.data()
        );

        if (res <= 0) return make_error(logic_error, "Font atlas size is too small.");

        auto rgba_pixels =
            std::vector<u8>(static_cast<std::size_t>(atlas_width * atlas_height * 4));
        for (const auto i : std::views::indices(atlas_pixels.size())) {
            const auto alpha         = atlas_pixels[i];
            rgba_pixels[(i * 4) + 0] = 255;
            rgba_pixels[(i * 4) + 1] = 255;
            rgba_pixels[(i * 4) + 2] = 255;
            rgba_pixels[(i * 4) + 3] = alpha;
        }

        auto f = font{make_texture(atlas_width, atlas_height, rgba_pixels.data())};

        for (const auto i : std::views::indices(96u)) {
            const auto& b         = baked_chars[i];
            const auto  codepoint = static_cast<char32_t>(32 + i);

            auto g = glyph{
                .uv_rect =
                    uv_rectangle{
                        .x      = static_cast<f32>(b.x0) / static_cast<f32>(atlas_width),
                        .y      = static_cast<f32>(b.y0) / static_cast<f32>(atlas_height),
                        .width  = static_cast<f32>(b.x1 - b.x0) / static_cast<f32>(atlas_width),
                        .height = static_cast<f32>(b.y1 - b.y0) / static_cast<f32>(atlas_height)
                    },
                .size =
                    extent{
                        .width  = static_cast<f32>(b.x1 - b.x0),
                        .height = static_cast<f32>(b.y1 - b.y0)
                    },
                .bearing = vec2{.x = b.xoff, .y = b.yoff},
                .advance = b.xadvance
            };
            f.glyphs_[codepoint] = g;
        }

        return f;
    }

    /**
     * @brief 指定したコードポイントに対応するグリフ情報を取得します。
     *
     * @param codepoint 取得対象のコードポイント（UTF-32 文字）
     * @return std::optional<const glyph&>
     * グリフ情報が存在する場合はその参照、存在しない場合は std::nullopt
     */
    [[nodiscard]] auto glyph_of_point(const char32_t codepoint) const noexcept
        -> std::optional<const glyph&> {
        const auto it = glyphs_.find(codepoint);
        if (it != glyphs_.end()) return it->second;
        return std::nullopt;
    }

    /**
     * @brief フォントアトラスのテクスチャ参照を取得します。
     *
     * @return const texture& アトラス表示用のテクスチャ参照
     */
    [[nodiscard]] auto setting_texture() const noexcept -> const texture& { return atlas_texture_; }

  private:
    explicit font(texture tex) noexcept : atlas_texture_(std::move(tex)) {}

    std::unordered_map<char32_t, glyph> glyphs_;
    texture                             atlas_texture_;
};

/**
 * @brief ファイルパスから font インスタンスの生成を試みるフリーのファクトリ関数
 *
 * @param path フォントファイルのパス
 * @param atlas_width アトラスの幅
 * @param atlas_height アトラスの高さ
 * @return std::expected<font, error> 成功時は font インスタンス、失敗時はエラー情報
 */
[[nodiscard]] inline auto try_make_font(
    const std::filesystem::path& path, const u32 atlas_width = 1024, const u32 atlas_height = 1024
) noexcept -> std::expected<font, error> {
    return font::try_make(path, atlas_width, atlas_height);
}

/**
 * @brief メモリバイナリから font インスタンスの生成を試みるフリーのファクトリ関数
 *
 * @param buffer フォントのバイナリデータ
 * @param atlas_width アトラスの幅
 * @param atlas_height アトラスの高さ
 * @return std::expected<font, error> 成功時は font インスタンス、失敗時はエラー情報
 */
[[nodiscard]] inline auto try_make_font(
    const std::span<const u8> buffer       = detail::default_font_binary,
    const u32                 atlas_width  = 1024,
    const u32                 atlas_height = 1024
) noexcept -> std::expected<font, error> {
    return font::try_make(buffer, atlas_width, atlas_height);
}
}  // namespace rin