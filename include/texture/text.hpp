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

    static constexpr auto base_font_size = 48.f;

  public:
    [[nodiscard]] static auto make(const font& font_ref) noexcept -> text { return text{font_ref}; }

    [[nodiscard]] auto string() const noexcept -> std::string_view {
        return tex_str.visit([](auto&& str) noexcept -> std::string_view { return str; });
    }
    [[nodiscard]] auto position() const noexcept -> vec2 { return vertices_.position(); }
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }
    [[nodiscard]] auto setting_font() const noexcept -> std::optional<const font&> { return font_; }
    [[nodiscard]] auto size() const noexcept -> f32 { return size_; }

    void string(const string_literal auto& str) noexcept {
        dirty_  = true;
        tex_str = std::string_view{str};
    }
    void string(const std::string_view str) noexcept {
        dirty_  = true;
        tex_str = std::string{str};
    }
    void position(const vec2& pos) noexcept { position(pos.x, pos.y); }
    void position(const f32 x, const f32 y) noexcept { vertices_.position(x, y); }
    void color(const rgba& col) noexcept { color(col.r, col.g, col.b, col.a); }
    void color(const u8 r, const u8 g, const u8 b, const u8 a = 0) noexcept {
        dirty_ = true;
        color_ = {.r = r, .g = g, .b = b, .a = a};
    }
    void setting_font(const font& font_obj) noexcept {
        dirty_ = true;
        font_.emplace(font_obj);
    }
    void size(const f32 s) noexcept {
        size_            = s;
        const auto scale = size_ / base_font_size;
        vertices_.scale(scale, scale);
    }

    [[nodiscard]] auto calc_vertices() noexcept -> const vertex_vector& {
        if (dirty_) update_vertices();
        dirty_ = false;
        return vertices_;
    }

    [[nodiscard]] auto bind_font(const u32 unit) noexcept -> bool {
        if (not font_) return false;
        font_->setting_texture().bind(unit);
        return true;
    }

  private:
    explicit text(const font& font_obj) noexcept : font_{font_obj} {}

    void update_vertices() noexcept {
        if (not font_) return;
        vertices_.clear();
        auto cursor_x = 0.f;
        auto cursor_y = 0.f;

        for (const auto c : string()) {
            if (c == '\n') {
                cursor_x = 0.f;
                cursor_y = base_font_size;
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

    vertex_vector              vertices_{make_vertex_vector(primitive_triangles)};
    str_t                      tex_str;
    rgba                       color_;
    std::optional<const font&> font_{std::nullopt};
    f32                        size_{0.f};
    bool                       dirty_{true};
};

[[nodiscard]] inline auto make_text(const font& font_ref) noexcept -> text {
    return text::make(font_ref);
}
}  // namespace rin