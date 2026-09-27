#pragma once

#include <cassert>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <optional>

#include "others/type.hpp"
#include "others/util.hpp"
#include "texture/texture.hpp"
#include "vertex.hpp"

namespace rin {
class sprite final {
  public:
    [[nodiscard]] static auto make(const texture& tex) noexcept -> sprite { return sprite{tex}; }

    [[nodiscard]] auto position() const noexcept -> vec2 { return position_; }
    [[nodiscard]] auto scale() const noexcept -> vec2 { return scale_; }
    [[nodiscard]] auto rotation() const noexcept -> f32 { return rotation_; }
    [[nodiscard]] auto origin() const noexcept -> vec2 { return origin_; }
    [[nodiscard]] auto color() const noexcept -> rgba { return color_; }
    [[nodiscard]] auto setting_texture() const noexcept -> std::optional<const texture&> {
        return tex_;
    }

    void position(const vec2 pos) noexcept { position(pos.x, pos.y); }
    void position(const f32 x, const f32 y) noexcept {
        dirty_    = true;
        position_ = {.x = x, .y = y};
    }
    void scale(const vec2 s) noexcept { scale_ = s; }
    void scale(const f32 x, const f32 y) noexcept { scale_ = {.x = x, .y = y}; }
    void rotation(const f32 r) noexcept { rotation_ = r; }
    void origin(const vec2 o) noexcept { origin(o.x, o.y); }
    void origin(const f32 x, const f32 y) noexcept {
        dirty_  = true;
        origin_ = {.x = x, .y = y};
    }
    void color(const rgba& col) noexcept { color(col.r, col.g, col.b, col.a); }
    void color(const u8 r, const u8 g, const u8 b, const u8 a = 255) noexcept {
        dirty_ = true;
        color_ = {.r = r, .g = g, .b = b, .a = a};
    }

    void setting_texture(const texture& tex) noexcept {
        dirty_ = true;
        tex_.emplace(tex);
        uv_rect_ = uv_rectangle{
            .x = uv_rect_.x, .y = uv_rect_.y, .width = tex.size().width, .height = tex.size().height
        };
    }

    [[nodiscard]] auto calc_transfrom_mat() const noexcept -> glm::mat4 {
        auto model = glm::translate(glm::mat4(1.f), glm::vec3{position_.x, position_.y, 0.f});
        if (rotation_ != 0.f) model = glm::rotate(model, rotation_, glm::vec3{0.f, 0.f, 1.f});
        model = glm::scale(model, glm::vec3{scale_.x, scale_.y, 1.f});
        return model;
    }

    [[nodiscard]] auto calc_vertices() noexcept -> const vertex_vector& {
        if (dirty_) update_vertices();
        dirty_ = false;
        return vertices_;
    }

  private:
    explicit sprite(const texture& tex) noexcept { setting_texture(tex); }

    void update_vertices() noexcept {
        const auto tex_w = tex_->size().width;
        const auto tex_h = tex_->size().height;

        const auto u0 = uv_rect_.x / tex_w;
        const auto v0 = uv_rect_.y / tex_h;
        const auto u1 = (uv_rect_.x + uv_rect_.width) / tex_w;
        const auto v1 = (uv_rect_.y + uv_rect_.height) / tex_h;

        const auto w = uv_rect_.width;
        const auto h = uv_rect_.height;

        const auto p0 = vec2{.x = -origin_.x, .y = -origin_.y};
        const auto p1 = vec2{.x = -origin_.x + w, .y = -origin_.y};
        const auto p2 = vec2{.x = -origin_.x + w, .y = -origin_.y + h};
        const auto p3 = vec2{.x = -origin_.x, .y = -origin_.y + h};

        vertices_.emplace_back(p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(p1, uv{.u = u1, .v = v0}, color_);
        vertices_.emplace_back(p2, uv{.u = u1, .v = v1}, color_);

        vertices_.emplace_back(p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(p2, uv{.u = u1, .v = v1}, color_);
        vertices_.emplace_back(p3, uv{.u = u0, .v = v1}, color_);
    }

    vertex_vector                 vertices_{make_vertex_vector(primitive_triangles)};
    std::optional<const texture&> tex_;
    uv_rectangle                  uv_rect_{};
    rgba                          color_{white};
    vec2                          position_;
    vec2                          scale_{.x = 1.f, .y = 1.f};
    vec2                          origin_;
    f32                           rotation_{0.f};
    bool                          dirty_{true};
};  // namespace rin

[[nodiscard]] inline auto make_sprite(const texture& tex) noexcept -> sprite {
    return sprite::make(tex);
}
}  // namespace rin