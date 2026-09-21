#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "type.hpp"

namespace rin {
struct Camera final {
  public:
    explicit constexpr Camera(const f32 width, const f32 height) noexcept
        : viewport_width_{width}, viewport_height_{height} {}

    [[nodiscard]] constexpr auto position() const noexcept -> glm::vec2 { return position_; }
    [[nodiscard]] constexpr auto zoom() const noexcept -> f32 { return zoom_; }
    [[nodiscard]] constexpr auto calc_view_position_mat() const noexcept -> glm::mat4 {
        const auto half_w = (viewport_width_ * 0.5f) / zoom_;
        const auto half_h = (viewport_height_ * 0.5f) / zoom_;

        const auto projection = glm::ortho(-half_w, half_w, -half_h, half_h, -1.f, 1.f);

        const auto view = glm::translate(glm::mat4(1.f), glm::vec3(-position_, 0.f));

        return projection * view;
    }

    constexpr void position(const glm::vec2 position) noexcept { position_ = position; }
    constexpr void position(const f32 x, const f32 y) noexcept { position_ = {x, y}; }
    constexpr void zoom(const f32 zoom) noexcept { zoom_ = (zoom > 0.f) ? zoom : 0.1f; }
    constexpr void view_port(const f32 width, const f32 height) noexcept {
        viewport_width_  = width;
        viewport_height_ = height;
    }

  private:
    glm::vec2 position_{0.f, 0.f};
    f32       zoom_{1.f};
    f32       viewport_width_;
    f32       viewport_height_;
};
}  // namespace rin