#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "rin/color.hpp"
#include "rin/detail/others.hpp"
#include "rin/extent.hpp"
#include "rin/font.hpp"
#include "rin/types.hpp"
#include "rin/uv.hpp"
#include "rin/vec2.hpp"
#include "rin/vertex_vector.hpp"

/**
 * @file text.hpp
 * @brief 文字列描画オブジェクトおよび頂点データの管理を行うモジュール
 */

namespace rin {
/**
 * @class text
 * @brief 文字列、位置、カラー、フォント参照を保持し、描画用頂点を生成・管理するクラス
 *
 * @warning 本クラスは内部で `font` への参照（`std::reference_wrapper`）を保持します。
 * 参照先の `font` インスタンスが破棄された後にアクセスすると未定義動作となるため、
 * `text` の生存期間が `font` の生存期間を超えないように注意してください。
 */
class text final {
    using str_t = std::variant<std::string_view, std::string>;

  public:
    /**
     * @brief フォントを指定して text インスタンスを生成します。
     *
     * @param font_ref 参照するフォント（インスタンスの生存期間に注意してください）
     * @return text 生成された text インスタンス
     */
    [[nodiscard]] static auto make(const font& font_ref) noexcept -> text { return text{font_ref}; }

    /**
     * @brief 設定されている文字列を取得します。
     * @return std::string_view 文字列ビュー
     */
    [[nodiscard]] auto string() const noexcept -> std::string_view {
        return tex_str.visit([](auto&& str) noexcept -> std::string_view { return str; });
    }

    /**
     * @brief 現在の位置座標を取得します。
     * @return vec2 位置座標
     */
    [[nodiscard]] auto position() const noexcept -> vec2 { return vertices_.position(); }

    /**
     * @brief 現在のカラーを取得します。
     * @return rgba カラー情報
     */
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }

    /**
     * @brief 参照しているフォントを取得します。
     * @return const font& フォントへの参照
     */
    [[nodiscard]] auto setting_font() const noexcept -> const font& { return font_; }

    /**
     * @brief 現在のスケールを取得します。
     * @return vec2 スケール値
     */
    [[nodiscard]] auto scale() const noexcept -> vec2 { return vertices_.scale(); }

    /**
     * @brief 文字列リテラルを非所有で設定します。
     * @tparam T 文字列リテラル型
     * @param str 文字列リテラル
     * @return 自身の参照
     */
    template <typename T>
        requires detail::is_string_literal_v<T>
    auto string(const T& str) noexcept -> text& {
        vertices_dirty_ = true;
        extent_dirty_   = true;
        tex_str         = std::string_view{str};
        return *this;
    }

    /**
     * @brief 文字列（string_view）を設定します。
     * @param str 設定する文字列ビュー
     * @return 自身の参照
     */
    auto string(const std::string_view str) noexcept -> text& {
        vertices_dirty_ = true;
        extent_dirty_   = true;
        tex_str         = std::string{str};
        return *this;
    }

    /**
     * @brief 数値（整数・浮動小数点数）を文字列に変換して設定します。
     * @tparam T 整数または浮動小数点数型
     * @param num 設定する数値
     * @return 自身の参照
     */
    template <typename T>
        requires std::integral<T> or std::floating_point<T>
    auto string(T num) noexcept -> text& {
        return string(std::to_string(num));
    }

    /**
     * @brief 位置座標を設定します。
     * @param pos 位置座標
     * @return 自身の参照
     */
    auto position(const vec2 pos) noexcept -> text& { return position(pos.x, pos.y); }

    /**
     * @brief 位置座標（X, Y成分）を設定します。
     * @param x X座標
     * @param y Y座標
     * @return 自身の参照
     */
    auto position(const f32 x, const f32 y) noexcept -> text& {
        vertices_.position(x, y);
        return *this;
    }

    /**
     * @brief 原点（原点オフセット）を設定します。
     * @param o 原点座標
     * @return 自身の参照
     */
    auto origin(const vec2 o) noexcept -> text& { return origin(o.x, o.y); }

    /**
     * @brief 原点（X, Y成分）を設定します。
     * @param x 原点X座標
     * @param y 原点Y座標
     */
    auto origin(const f32 x, const f32 y) noexcept -> text& {
        const auto scale   = vertices_.scale();
        const auto local_x = (scale.x != 0.f) ? x / scale.x : x;
        const auto local_y = (scale.y != 0.f) ? y / scale.y : y;
        vertices_.origin(local_x, local_y);
        return *this;
    }

    /**
     * @brief 描画カラーを設定します。
     * @param col カラー情報（rgba）
     * @return 自身の参照
     */
    auto color(const rgba& col) noexcept -> text& { return color(col.r, col.g, col.b, col.a); }

    /**
     * @brief 描画カラー（各チャンネル値）を設定します。
     * @param r 赤成分 (0〜255)
     * @param g 緑成分 (0〜255)
     * @param b 青成分 (0〜255)
     * @param a アルファ成分 (0〜255、デフォルト値: 255)
     * @return 自身の参照
     */
    auto color(const u8 r, const u8 g, const u8 b, const u8 a = 255) noexcept -> text& {
        vertices_dirty_ = true;
        color_          = {.r = r, .g = g, .b = b, .a = a};
        return *this;
    }

    /**
     * @brief 参照するフォントを変更・再設定します。
     *
     * @warning 渡すフォントの生存期間がこの text インスタンスより長くなるようにしてください。
     * @param font_ref 設定するフォントの参照
     * @return 自身の参照
     */
    auto setting_font(const font& font_ref) noexcept -> text& {
        vertices_dirty_ = true;
        extent_dirty_   = true;
        font_           = std::cref(font_ref);
        return *this;
    }

    /**
     * @brief スケール（拡大率）を設定します。
     * @param s スケール値
     * @return 自身の参照
     */
    auto scale(const vec2 s) noexcept -> text& { return scale(s.x, s.y); }

    /**
     * @brief スケール（X, Y成分）を設定します。
     * @param x X方向スケール
     * @param y Y方向スケール
     * @return 自身の参照
     */
    auto scale(const f32 x, const f32 y) noexcept -> text& {
        vertices_.scale(x, y);
        return *this;
    }

    /**
     * @brief 現在の文字列とスケールに応じた描画サイズ（幅・高さ）を計算・取得します。
     * @details キャッシュを書き換えるため非constです。計算量は償却定数時間
     * @return extent 計算されたサイズ
     */
    [[nodiscard]] auto calc_extent() noexcept -> extent {
        const auto base_extent = calc_base_extent();
        const auto scale       = vertices_.scale();
        return extent{.width = base_extent.width * scale.x, .height = base_extent.height * scale.y};
    }

    /**
     * @brief テキスト描画に必要な最新の頂点情報を計算・取得します。
     * @details キャシュを書き換えるため非constです。計算量は償却定数時間
     * @return const vertex_vector& 算出された頂点データの参照
     */
    [[nodiscard]] auto calc_vertices() noexcept -> const vertex_vector& {
        if (vertices_dirty_) update_vertices();
        vertices_dirty_ = false;
        return vertices_;
    }

    /**
     * @brief フォントアトラスのテクスチャを指定されたテクスチャユニットにバインドします。
     * @warning これは内部で使用します。呼び出しは行わないでください。
     * @param unit バインド先のスロット番号
     */
    void bind(const u32 unit) noexcept { font_.get().setting_texture().bind(unit); }

  private:
    explicit text(const font& font_obj) noexcept : font_{font_obj} {}

    [[nodiscard]] auto calc_base_extent() noexcept -> extent {
        if (not extent_dirty_) return base_extent_;

        auto max_width      = 0.f;
        auto current_line_w = 0.f;

        constexpr auto line_height = default_font_size;
        auto           line_count  = 1;

        for (const auto c : string()) {
            if (c == '\n') {
                max_width      = std::max(max_width, current_line_w);
                current_line_w = 0.f;
                line_count++;
                continue;
            }
            if (c == '\r') {
                continue;  // '\r\n' 対策として '\r' は無視
            }

            const auto g = font_.get().glyph_of_point(static_cast<char32_t>(c));
            if (not g) continue;

            current_line_w += g->advance;
        }

        max_width = std::max(max_width, current_line_w);

        const auto total_height = static_cast<f32>(line_count) * line_height;

        base_extent_  = extent{.width = max_width, .height = total_height};
        extent_dirty_ = false;

        return base_extent_;
    }

    void update_vertices() noexcept {
        vertices_.clear();
        auto cursor_x = 0.f;
        auto cursor_y = 0.f;

        for (const auto c : string()) {
            if (c == '\n') {
                cursor_x = 0.f;
                cursor_y = default_font_size;
                continue;
            }

            const auto g = font_.get().glyph_of_point(static_cast<char32_t>(c));
            if (not g) continue;

            const auto x0             = std::floor(cursor_x + g->bearing.x);
            const auto x1             = x0 + g->size.width;
            const auto y1             = std::floor(cursor_y - g->bearing.y);  // 文字の上端
            const auto x0_y0_bottom_y = y1 - g->size.height;                  // 文字の下端 (y0)
            const auto y0             = x0_y0_bottom_y;

            const auto u0 = g->uv_rect.x;
            const auto u1 = g->uv_rect.x + g->uv_rect.width;
            const auto v0 = g->uv_rect.y;                      // 画像上の「上端」
            const auto v1 = g->uv_rect.y + g->uv_rect.height;  // 画像上の「下端」

            vertices_.emplace_back(vec2{.x = x0, .y = y0}, uv{.u = u0, .v = v1}, color_);
            vertices_.emplace_back(vec2{.x = x1, .y = y0}, uv{.u = u1, .v = v1}, color_);
            vertices_.emplace_back(vec2{.x = x1, .y = y1}, uv{.u = u1, .v = v0}, color_);

            vertices_.emplace_back(vec2{.x = x0, .y = y0}, uv{.u = u0, .v = v1}, color_);
            vertices_.emplace_back(vec2{.x = x1, .y = y1}, uv{.u = u1, .v = v0}, color_);
            vertices_.emplace_back(vec2{.x = x0, .y = y1}, uv{.u = u0, .v = v0}, color_);

            cursor_x += g->advance;
        }
    }

    vertex_vector                      vertices_{primitive_triangles};
    str_t                              tex_str{std::string_view{""}};
    rgba                               color_;
    extent                             base_extent_{};
    std::reference_wrapper<const font> font_;
    bool                               vertices_dirty_{true};
    bool                               extent_dirty_{true};
};

/**
 * @brief text インスタンスを生成するフリーのファクトリ関数
 *
 * @warning 参照する `font` インスタンスが `text` より長く生存している必要があります。
 * @param font_ref 参照するフォント
 * @return text 生成された text インスタンス
 */
[[nodiscard]] inline auto make_text(const font& font_ref) noexcept -> text {
    return text::make(font_ref);
}
}  // namespace rin