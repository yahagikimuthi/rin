#pragma once

#include <algorithm>
#include <cmath>
#include <expected>
#include <filesystem>
#include <utility>

#include "rin/detail/graphics.hpp"

#define STB_IMAGE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "rin/detail/stb_image.h"
#pragma GCC diagnostic pop

#include "rin/error.hpp"
#include "rin/extent.hpp"
#include "rin/types.hpp"

/**
 * @file texture.hpp
 * @brief OpenGL テクスチャリソースの作成・管理を行うモジュール
 */

namespace rin {

/**
 * @struct uv_rectangle
 * @brief テクスチャを表現するためのUV構造体
 */
struct uv_rectangle final {
    /// テクスチャ上における領域の起点となるx座標
    f32 x{};

    /// テクスチャ上における領域の起点となるy座標
    f32 y{};

    /// 描画や切り出しに使う領域の幅と
    f32 width{};

    /// 描画や切り出しに使う領域の高さ
    f32 height{};
};

/**
 * @class texture
 * @brief OpenGL 2Dテクスチャリソースを RAII パターンで管理するクラス
 *
 * @details コピー不可・ムーブ可能であり、デストラクタ呼び出し時に自動的に OpenGL
 * テクスチャオブジェクトを破棄します。 直接のコンストラクタ呼び出しは行わず、静的ファクトリ関数
 * `make`, `try_make` または対応するフリー関数を使用してください。
 */
class texture final {
  public:
    /**
     * @brief ピクセルデータ配列から texture インスタンスを生成します。
     *
     * @param width テクスチャの幅（ピクセル）
     * @param height テクスチャの高さ（ピクセル）
     * @param pixels RGBAフォーマットのピクセル配列データへのポインタ（null 許容）
     * @return texture 生成された texture インスタンス
     */
    [[nodiscard]] static auto make(
        const u32 width, const u32 height, const u8* const pixels
    ) noexcept -> texture {
        return texture{width, height, pixels};
    }

    /**
     * @brief 画像ファイルパスから texture インスタンスの生成を試みます。
     *
     * @param path 画像ファイルのパス
     * @return std::expected<texture, error> 成功した場合は texture
     * インスタンス、失敗した場合はエラー情報
     */
    [[nodiscard]] static auto try_make(const std::filesystem::path& path) noexcept
        -> std::expected<texture, error> {
        stbi_set_flip_vertically_on_load(static_cast<int>(true));

        auto width    = 0;
        auto height   = 0;
        auto channels = 0;

        auto* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
        if (data == nullptr) return make_error(logic_error, "Failed to load texture file.");

        auto tex = texture{static_cast<u32>(width), static_cast<u32>(height), data};

        stbi_image_free(data);
        return tex;
    }

    texture(const texture&) noexcept                    = delete;
    auto operator=(const texture&) noexcept -> texture& = delete;

    texture(texture&& other) noexcept
        : id_{std::exchange(other.id_, 0)}, size_{std::exchange(other.size_, extent{})} {}
    auto operator=(texture&& other) noexcept -> texture& {
        if (this == &other) return *this;

        destroy();

        id_   = std::exchange(other.id_, 0);
        size_ = std::exchange(other.size_, extent{});
        return *this;
    }
    ~texture() noexcept { destroy(); }

    /**
     * @brief テクスチャのサイズ（幅・高さ）を取得します。
     * @return extent テクスチャサイズ
     */
    [[nodiscard]] auto size() const noexcept -> extent { return size_; }

    /**
     * @brief テクスチャを指定したテクスチャユニットにバインドします。
     * @attention これは内部で使用します。呼び出しは行わないでください
     * @param unit バインド先のスロット番号
     */
    void bind(const u32 unit) const noexcept { glBindTextureUnit(unit, id_); }

  private:
    explicit texture(const u32 width, const u32 height, const u8* const pixels) noexcept
        : size_{.width = static_cast<f32>(width), .height = static_cast<f32>(height)} {
        glCreateTextures(GL_TEXTURE_2D, 1, &id_);

        // ミップマップレベルを生成
        const auto mip_levels =
            static_cast<GLsizei>(std::floor(std::log2(std::max(width, height)))) + 1;

        glTextureStorage2D(
            id_, mip_levels, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height)
        );

        if (pixels != nullptr) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureSubImage2D(
                id_,
                0,  // Mip Level 0 (原寸)
                0,
                0,
                static_cast<GLsizei>(width),
                static_cast<GLsizei>(height),
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                pixels
            );

            glGenerateTextureMipmap(id_);
        }

        glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // ミップマップを設定
        glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void destroy() noexcept {
        if (id_ != 0) {
            glDeleteTextures(1, &id_);
            id_ = 0;
        }
    }

    GLuint id_{};
    extent size_{};
};

/**
 * @brief ピクセル配列データから texture インスタンスを生成するフリーのファクトリ関数
 *
 * @param width テクスチャの幅（ピクセル）
 * @param height テクスチャの高さ（ピクセル）
 * @param pixels RGBAピクセル配列へのポインタ
 * @return texture 生成された texture インスタンス
 */
[[nodiscard]] inline auto make_texture(
    const u32 width, const u32 height, const u8* const pixels
) noexcept -> texture {
    return texture::make(width, height, pixels);
}

/**
 * @brief 画像ファイルから texture インスタンスの生成を試みるフリーのファクトリ関数
 *
 * @param path 画像ファイルのパス
 * @return std::expected<texture, error> 成功時は texture インスタンス、失敗時はエラー情報
 */
[[nodiscard]] inline auto try_make_texture(const std::filesystem::path& path) noexcept
    -> std::expected<texture, error> {
    return texture::try_make(path);
}
}  // namespace rin