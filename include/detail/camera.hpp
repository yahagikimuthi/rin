#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "extent.hpp"
#include "type.hpp"
#include "vec2.hpp"

namespace rin::detail {
struct camera final {
  public:
    explicit camera(const f32 window_width, const f32 window_height) noexcept
        : window_size_{.width = window_width, .height = window_height} {}
    explicit camera(const extent window_size) noexcept
        : camera(window_size.width, window_size.height) {}

    [[nodiscard]] constexpr auto position() const noexcept -> vec2 { return position_; }
    [[nodiscard]] constexpr auto zoom() const noexcept -> f32 { return zoom_; }

    [[nodiscard]] constexpr auto calc_view_position_mat(
        const extent& virtual_window_size
    ) const noexcept -> glm::mat4 {
        const auto virtual_w = virtual_window_size.width;
        const auto virtual_h = virtual_window_size.height;

        const auto view_w = virtual_w / zoom_;
        const auto view_h = virtual_h / zoom_;

        const auto projection = glm::ortho(0.0f, view_w, 0.0f, view_h, -1.0f, 1.0f);

        const auto view =
            glm::translate(glm::mat4(1.0f), glm::vec3(-position_.x, -position_.y, 0.0f));

        return projection * view;
    }

    constexpr void position(const vec2 position) noexcept { position_ = position; }
    constexpr void position(const f32 x, const f32 y) noexcept { position_ = {.x = x, .y = y}; }
    constexpr void zoom(const f32 zoom) noexcept { zoom_ = (zoom > 0.f) ? zoom : 0.1f; }
    constexpr void window_size(const f32 width, const f32 height) noexcept {
        window_size_ = {.width = width, .height = height};
    }

  private:
    vec2   position_{.x = 0.f, .y = 0.f};
    extent window_size_;
    f32    zoom_{1.f};
};
}  // namespace rin::detail