#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
struct camera final {
  public:
    [[nodiscard]] static auto make(const f32 window_width, const f32 window_height) noexcept
        -> camera {
        return camera{window_width, window_height};
    }
    [[nodiscard]] static auto make(const extent window_size) noexcept -> camera {
        return make(window_size.width, window_size.height);
    }

    [[nodiscard]] constexpr auto position() const noexcept -> vec2 { return position_; }
    [[nodiscard]] constexpr auto zoom() const noexcept -> f32 { return zoom_; }

    [[nodiscard]] constexpr auto calc_view_position_mat() const noexcept -> glm::mat4 {
        constexpr auto virtual_w = 800.0f;
        constexpr auto virtual_h = 600.0f;

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
    explicit constexpr camera(const f32 window_width, const f32 window_height) noexcept
        : window_size_{.width = window_width, .height = window_height} {}

    vec2   position_{.x = 0.f, .y = 0.f};
    extent window_size_;
    f32    zoom_{1.f};
};

[[nodiscard]] inline auto make_camera(const f32 window_width, const f32 window_height) noexcept
    -> camera {
    return camera::make(window_width, window_height);
}
[[nodiscard]] inline auto make_camera(const extent window_size) noexcept -> camera {
    return camera::make(window_size);
}
}  // namespace rin