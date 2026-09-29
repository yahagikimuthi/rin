#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "others/setting.hpp"
#include "others/type.hpp"
#include "others/util.hpp"
#include "texture/font.hpp"
#include "vertex.hpp"

namespace rin {
class text final {
    using str_t = std::variant<std::string_view, std::string>;

  public:
    [[nodiscard]] static auto make(const font& font_ref) noexcept -> text { return text{font_ref}; }

    [[nodiscard]] auto string() const noexcept -> std::string_view {
        return tex_str.visit([](auto&& str) noexcept -> std::string_view { return str; });
    }
    [[nodiscard]] auto position() const noexcept -> vec2 { return vertices_.position(); }
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }
    [[nodiscard]] auto setting_font() const noexcept -> std::optional<const font&> { return font_; }
    [[nodiscard]] auto scale() const noexcept -> vec2 { return vertices_.scale(); }

    void string(const string_literal auto& str) noexcept {
        vertices_dirty_ = true;
        extent_dirty_   = true;
        tex_str         = std::string_view{str};
    }
    void string(const std::string_view str) noexcept {
        vertices_dirty_ = true;
        extent_dirty_   = true;
        tex_str         = std::string{str};
    }
    void position(const vec2 pos) noexcept { position(pos.x, pos.y); }
    void position(const f32 x, const f32 y) noexcept { vertices_.position(x, y); }
    void origin(const vec2 o) noexcept { origin(o.x, o.y); }
    void origin(const f32 x, const f32 y) noexcept {
        const auto scale   = vertices_.scale();
        const auto local_x = (scale.x != 0.f) ? x / scale.x : x;
        const auto local_y = (scale.y != 0.f) ? y / scale.y : y;
        vertices_.origin(local_x, local_y);
    }
    void color(const rgba& col) noexcept { color(col.r, col.g, col.b, col.a); }
    void color(const u8 r, const u8 g, const u8 b, const u8 a = 0) noexcept {
        vertices_dirty_ = true;
        color_          = {.r = r, .g = g, .b = b, .a = a};
    }
    void setting_font(const font& font_obj) noexcept {
        vertices_dirty_ = true;
        extent_dirty_   = true;
        font_.emplace(font_obj);
    }
    void scale(const vec2 s) noexcept { vertices_.scale(s); }
    void scale(const f32 x, const f32 y) noexcept { vertices_.scale(x, y); }

    [[nodiscard]] auto calc_extent() noexcept -> extent {
        const auto base_extent = calc_base_extent();
        const auto scale       = vertices_.scale();
        return extent{.width = base_extent.width * scale.x, .height = base_extent.height * scale.y};
    }

    [[nodiscard]] friend auto calc_vertices(text& self) noexcept -> const vertex_vector& {
        if (self.vertices_dirty_) self.update_vertices();
        self.vertices_dirty_ = false;
        return self.vertices_;
    }

    [[nodiscard]] friend auto bind_font(const text& self, const u32 unit) noexcept -> bool {
        if (not self.font_) return false;
        bind(self.font_->setting_texture(), unit);
        return true;
    }

  private:
    explicit text(const font& font_obj) noexcept : font_{font_obj} {}

    [[nodiscard]] auto calc_base_extent() noexcept -> extent {
        if (not font_) return extent{};

        if (not extent_dirty_) return base_extent_;

        auto max_width      = 0.f;
        auto current_line_w = 0.f;

        const auto line_height = default_font_size;
        auto       line_count  = 1;

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

            const auto g = font_->glyph_of_point(static_cast<char32_t>(c));
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
        if (not font_) return;
        vertices_.clear();
        auto cursor_x = 0.f;
        auto cursor_y = 0.f;

        for (const auto c : string()) {
            if (c == '\n') {
                cursor_x = 0.f;
                cursor_y = default_font_size;
                continue;
            }

            const auto g = font_->glyph_of_point(static_cast<char32_t>(c));
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

    vertex_vector              vertices_{make_vertex_vector(primitive_triangles)};
    str_t                      tex_str{std::string_view{""}};
    rgba                       color_;
    extent                     base_extent_{};
    std::optional<const font&> font_{std::nullopt};
    bool                       vertices_dirty_{true};
    bool                       extent_dirty_{true};
};

[[nodiscard]] inline auto make_text(const font& font_ref) noexcept -> text {
    return text::make(font_ref);
}
}  // namespace rin