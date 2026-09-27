#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "others/type.hpp"
#include "others/util.hpp"
#include "texture/font.hpp"
#include "vertex.hpp"

namespace rin {
class text final {
    using str_t = std::variant<std::string, std::string_view>;

    friend class renderer;

  public:
    [[nodiscard]] static auto make(const font& font_ref) noexcept -> text { return text{font_ref}; }

    [[nodiscard]] auto string() const noexcept -> std::string_view {
        return tex_str.visit([](auto&& str) noexcept -> std::string_view { return str; });
    }
    [[nodiscard]] auto position() const noexcept -> vec2 { return position_; }
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }
    [[nodiscard]] auto setting_font() const noexcept -> std::optional<const font&> { return font_; }

    void string(const string_literal auto& str) noexcept {
        dirty_  = true;
        tex_str = std::string_view{str};
    }
    void string(const std::string_view str) noexcept {
        dirty_  = true;
        tex_str = std::string{str};
    }
    void position(const vec2& pos) noexcept { position(pos.x, pos.y); }
    void position(const f32 x, const f32 y) noexcept {
        dirty_    = true;
        position_ = {.x = x, .y = y};
    }
    void color(const rgba& col) noexcept { color(col.r, col.g, col.b, col.a); }
    void color(const u8 r, const u8 g, const u8 b, const u8 a = 0) noexcept {
        dirty_ = true;
        color_ = {.r = r, .g = g, .b = b, .a = a};
    }
    void setting_font(const font& font_obj) noexcept {
        dirty_ = true;
        font_.emplace(font_obj);
    }

    [[nodiscard]] auto calc_vertices() noexcept -> const vertex_vector& {
        if (dirty_) update_vertices();
        dirty_ = false;
        return vertices_;
    }

  private:
    void update_vertices() noexcept {
        if (not font_) return;
        vertices_.clear();
        auto cursor_x = position_.x;
        auto cursor_y = position_.y;

        for (const auto c : string()) {
            if (c == '\n') {
                cursor_x = position_.x;
                cursor_y = font_->size();
                continue;
            }

            const auto g = font_->glyph_of_point(static_cast<char32_t>(c));
            if (not g) continue;

            const auto x0 = std::floor(cursor_x + g->bearing.x);
            const auto y0 = std::floor(cursor_y + g->bearing.y);
            const auto x1 = x0 + g->size.width;
            const auto y1 = y0 + g->size.height;

            const auto u0 = g->uv_rect.x;
            const auto v0 = g->uv_rect.y;
            const auto u1 = g->uv_rect.x + g->uv_rect.width;
            const auto v1 = g->uv_rect.y + g->uv_rect.height;

            vertices_.emplace_back(vec2{.x = x0, .y = y0}, uv{.u = u0, .v = v0}, color_);
            vertices_.emplace_back(vec2{.x = x1, .y = y0}, uv{.u = u1, .v = v0}, color_);
            vertices_.emplace_back(vec2{.x = x1, .y = y1}, uv{.u = u1, .v = v1}, color_);

            vertices_.emplace_back(vec2{.x = x0, .y = y0}, uv{.u = u0, .v = v0}, color_);
            vertices_.emplace_back(vec2{.x = x1, .y = y1}, uv{.u = u1, .v = v1}, color_);
            vertices_.emplace_back(vec2{.x = x0, .y = y1}, uv{.u = u0, .v = v1}, color_);

            cursor_x += g->advance;
        }
    }

    explicit text(const font& font_obj) noexcept : font_{font_obj} {}
    vertex_vector              vertices_{make_vertex_vector(primitive_triangles)};
    str_t                      tex_str;
    rgba                       color_;
    vec2                       position_;
    std::optional<const font&> font_{std::nullopt};
    bool                       dirty_{true};
};

[[nodiscard]] inline auto make_text(const font& font_ref) noexcept -> text {
    return text::make(font_ref);
}
}  // namespace rin