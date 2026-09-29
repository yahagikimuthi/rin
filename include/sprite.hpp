#pragma once

#include <cassert>
#include <functional>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

#include "texture.hpp"
#include "type.hpp"
#include "util.hpp"
#include "vec2.hpp"
#include "vertex_vector.hpp"

namespace rin {
class sprite final {
  public:
    [[nodiscard]] static auto make(const texture& tex) noexcept -> sprite { return sprite{tex}; }

    [[nodiscard]] auto position() const noexcept -> vec2 { return vertices_.position(); }
    [[nodiscard]] auto scale() const noexcept -> vec2 { return vertices_.scale(); }
    [[nodiscard]] auto rotation() const noexcept -> f32 { return vertices_.rotation(); }
    [[nodiscard]] auto origin() const noexcept -> vec2 { return vertices_.origin(); }
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }
    [[nodiscard]] auto setting_texture() const noexcept -> const texture& { return tex_; }

    void position(const vec2 pos) noexcept { vertices_.position(pos); }
    void position(const f32 x, const f32 y) noexcept { vertices_.position(x, y); }
    void scale(const vec2 scale) noexcept { vertices_.scale(scale); }
    void scale(const f32 x, const f32 y) noexcept { vertices_.scale(x, y); }
    void rotation(const f32 radian) noexcept { vertices_.rotation(radian); }
    void origin(const vec2 origin) noexcept { vertices_.origin(origin); }
    void origin(const f32 x, const f32 y) noexcept { vertices_.origin(x, y); }
    void color(const rgba& col) noexcept { color(col.r, col.g, col.b, col.a); }
    void color(const u8 r, const u8 g, const u8 b, const u8 a = 255) noexcept {
        dirty_ = true;
        color_ = {.r = r, .g = g, .b = b, .a = a};
    }

    void setting_texture(const texture& tex) noexcept {
        dirty_   = true;
        tex_     = std::cref(tex);
        uv_rect_ = uv_rectangle{
            .x = uv_rect_.x, .y = uv_rect_.y, .width = tex.size().width, .height = tex.size().height
        };
    }

    [[nodiscard]] friend auto calc_vertices(sprite& self) noexcept -> const vertex_vector& {
        if (self.dirty_) self.update_vertices();
        self.dirty_ = false;
        return self.vertices_;
    }

  private:
    explicit sprite(const texture& tex) noexcept
        : tex_{std::cref(tex)},
          uv_rect_{.x = 0.f, .y = 0.f, .width = tex.size().width, .height = tex.size().height} {}

    void update_vertices() noexcept {
        const auto tex_w = tex_.get().size().width;
        const auto tex_h = tex_.get().size().height;

        const auto u0 = uv_rect_.x / tex_w;
        const auto v0 = uv_rect_.y / tex_h;
        const auto u1 = (uv_rect_.x + uv_rect_.width) / tex_w;
        const auto v1 = (uv_rect_.y + uv_rect_.height) / tex_h;

        const auto w = uv_rect_.width;
        const auto h = uv_rect_.height;

        const auto p0 = vec2{.x = 0.f, .y = 0.f};
        const auto p1 = vec2{.x = 0.f + w, .y = 0.f};
        const auto p2 = vec2{.x = 0.f + w, .y = 0.f + h};
        const auto p3 = vec2{.x = 0.f, .y = 0.f + h};

        vertices_.emplace_back(p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(p1, uv{.u = u1, .v = v0}, color_);
        vertices_.emplace_back(p2, uv{.u = u1, .v = v1}, color_);

        vertices_.emplace_back(p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(p2, uv{.u = u1, .v = v1}, color_);
        vertices_.emplace_back(p3, uv{.u = u0, .v = v1}, color_);
    }

    vertex_vector                         vertices_{primitive_triangles};
    std::reference_wrapper<const texture> tex_;
    uv_rectangle                          uv_rect_{};
    rgba                                  color_{white};
    bool                                  dirty_{true};
};  // namespace rin

[[nodiscard]] inline auto make_sprite(const texture& tex) noexcept -> sprite {
    return sprite::make(tex);
}
}  // namespace rin