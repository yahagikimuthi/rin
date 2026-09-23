#pragma once

#include "others/type.hpp"
#include "others/util.hpp"
#include "texture.hpp"
#include "vertex.hpp"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <optional>

namespace rin {
struct rect final {
    f32 x{};
    f32 y{};
    f32 width{};
    f32 height{};
};

class sprite final {
  public:
    void set_texture(const texture& tex, const bool reset_rect = true) noexcept {
        tex_.emplace(tex);
        if (reset_rect) {
            tex_rect_ =
                rect{.x = 0.f, .y = 0.f, .width = tex.size().width, .height = tex.size().height};
        }
        dirty_ = true;
    }

    void set_texture(const texture& tex, const rect& tex_rect) noexcept {
        tex_.emplace(tex);
        tex_rect_ = tex_rect;
        dirty_    = true;
    }

    void set_texture_rect(const rect& tex_rect) noexcept {
        tex_rect_ = tex_rect;
        dirty_    = true;
    }

    [[nodiscard]] auto get_vertices() noexcept -> const vertex_vector& {
        if (dirty_) {
            update_vertices();
            dirty_ = false;
        }
        return vertices_;
    }

  private:
    void update_vertices() noexcept {
        if (not tex_) return;

        vertices_.clear();
        const auto tex_w = tex_->size().width;
        const auto tex_h = tex_->size().height;

        const auto u0 = tex_rect_.x / tex_w;
        const auto v0 = tex_rect_.y / tex_h;
        const auto u1 = (tex_rect_.x + tex_rect_.width) / tex_w;
        const auto v1 = (tex_rect_.y + tex_rect_.height) / tex_h;

        const auto w = tex_rect_.width;
        const auto h = tex_rect_.height;

        const auto p0 = vec2{.x = -origin_.x, .y = -origin_.y};
        const auto p1 = vec2{.x = -origin_.x + w, .y = -origin_.y};
        const auto p2 = vec2{.x = -origin_.x + w, .y = -origin_.y + h};
        const auto p3 = vec2{.x = -origin_.x, .y = -origin_.y + h};

        auto transform = glm::mat4{1.f};
        transform      = glm::translate(transform, glm::vec3{position_.x, position_.y, 0.f});

        if (rotation_ != 0.f)
            transform = glm::rotate(transform, rotation_, glm::vec3{0.f, 0.f, 1.f});

        auto transform_pos = [&](vec2 p) noexcept -> vec2 {
            const auto v = transform * glm::vec4{p.x, p.y, 0.f, 1.f};
            return vec2{.x = v.x, .y = v.y};
        };

        const auto world_p0 = transform_pos(p0);
        const auto world_p1 = transform_pos(p1);
        const auto world_p2 = transform_pos(p2);
        const auto world_p3 = transform_pos(p3);

        vertices_.emplace_back(world_p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(world_p1, uv{.u = u1, .v = v0}, color_);
        vertices_.emplace_back(world_p2, uv{.u = u1, .v = v1}, color_);

        vertices_.emplace_back(world_p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(world_p2, uv{.u = u1, .v = v1}, color_);
        vertices_.emplace_back(world_p3, uv{.u = u0, .v = v1}, color_);
    }

    std::optional<const texture&> tex_;
    vertex_vector                 vertices_;
    rect                          tex_rect_;
    rgba                          color_;
    vec2                          position_;
    vec2                          origin_;
    f32                           rotation_;

    bool dirty_;
};
}  // namespace rin