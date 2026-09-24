#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <optional>

#include "others/type.hpp"
#include "others/util.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace rin {

class sprite final {
    struct rect final {
        f32 x{};
        f32 y{};
        f32 width{};
        f32 height{};
    };

  public:
    explicit sprite(const texture& tex) noexcept { settle_texture(tex); }

    void settle_texture(const texture& tex) noexcept {
        tex_.emplace(tex);
        tex_rect_ =
            rect{.x = 0.f, .y = 0.f, .width = tex.size().width, .height = tex.size().height};
        dirty_ = true;
    }

    [[nodiscard]] auto calc_vertices() noexcept -> const vertex_vector& {
        if (dirty_) {
            update_vertices();
            dirty_ = false;
        }
        return vertices_;
    }

    [[nodiscard]] auto settle_texture() const noexcept -> std::optional<const texture&> {
        return tex_;
    }

  private:
    void update_vertices() noexcept {
        if (not tex_) return;

        vertices_.clear();

        // UV座標正規化
        const auto tex_w = tex_->size().width;
        const auto tex_h = tex_->size().height;

        const auto u0 = tex_rect_.x / tex_w;
        const auto v0 = tex_rect_.y / tex_h;
        const auto u1 = (tex_rect_.x + tex_rect_.width) / tex_w;
        const auto v1 = (tex_rect_.y + tex_rect_.height) / tex_h;

        // 原点・サイズを考慮したローカル4角
        const auto w = tex_rect_.width;
        const auto h = tex_rect_.height;

        // 原点を引いたローカル座標 (0,0 は原点位置)
        const vec2 p0{.x = -origin_.x, .y = -origin_.y};
        const vec2 p1{.x = -origin_.x + w, .y = -origin_.y};
        const vec2 p2{.x = -origin_.x + w, .y = -origin_.y + h};
        const vec2 p3{.x = -origin_.x, .y = -origin_.y + h};

        // 回転・平行移動・スケールの変換行列を作成して各頂点を変換
        auto transform = glm::mat4{1.0f};
        transform      = glm::translate(transform, glm::vec3{position_.x, position_.y, 0.0f});
        if (rotation_ != 0.0f) {
            transform = glm::rotate(transform, rotation_, glm::vec3{0.0f, 0.0f, 1.0f});
        }
        transform = glm::scale(transform, glm::vec3{scale_.x, scale_.y, 1.0f});

        auto transform_pos = [&](vec2 p) -> vec2 {
            const auto v = transform * glm::vec4{p.x, p.y, 0.0f, 1.0f};
            return vec2{.x = v.x, .y = v.y};
        };

        const auto world_p0 = transform_pos(p0);
        const auto world_p1 = transform_pos(p1);
        const auto world_p2 = transform_pos(p2);
        const auto world_p3 = transform_pos(p3);

        // 三角形2個分（6頂点）を vertex_vector に追加
        vertices_.emplace_back(world_p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(world_p1, uv{.u = u1, .v = v0}, color_);
        vertices_.emplace_back(world_p2, uv{.u = u1, .v = v1}, color_);

        vertices_.emplace_back(world_p0, uv{.u = u0, .v = v0}, color_);
        vertices_.emplace_back(world_p2, uv{.u = u1, .v = v1}, color_);
        vertices_.emplace_back(world_p3, uv{.u = u0, .v = v1}, color_);
    }

    std::optional<const texture&> tex_;
    vertex_vector                 vertices_{primitive_triangles};
    rect                          tex_rect_;
    rgba                          color_{white};
    vec2                          position_;
    vec2                          scale_{.x = 1.f, .y = 1.f};
    vec2                          origin_;
    f32                           rotation_{0.f};
    bool                          dirty_{true};
};
}  // namespace rin