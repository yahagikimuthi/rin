#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
struct camera final {
  public:
    explicit constexpr camera(const f32 window_width, const f32 window_height) noexcept
        : window_size_{.width = window_width, .height = window_height} {}
    explicit constexpr camera(const extent window_size) noexcept : window_size_{window_size} {}

    template <typename Self>
    [[nodiscard]] constexpr auto position(this Self&& self) noexcept -> auto&& {
        return std::forward<Self>(self).position_;
    }
    template <typename Self>
    [[nodiscard]] constexpr auto zoom(this Self&& self) noexcept -> auto&& {
        return std::forward<Self>(self).zoom_;
    }
    [[nodiscard]] constexpr auto calc_view_position_mat() const noexcept -> glm::mat4 {
        const auto half_w = (window_size_.width * 0.5f) / zoom_;
        const auto half_h = (window_size_.height * 0.5f) / zoom_;

        const auto projection = glm::ortho(-half_w, half_w, -half_h, half_h, -1.f, 1.f);

        const auto view =
            glm::translate(glm::mat4(1.f), glm::vec3(-static_cast<glm::vec2>(position_), 0.f));

        return projection * view;
    }

    constexpr void position(const vec2 position) noexcept { position_ = position; }
    constexpr void position(const f32 x, const f32 y) noexcept { position_ = {.x = x, .y = y}; }
    constexpr void zoom(const f32 zoom) noexcept { zoom_ = (zoom > 0.f) ? zoom : 0.1f; }
    constexpr void view_port(const f32 width, const f32 height) noexcept {
        window_size_ = {.width = width, .height = height};
    }

  private:
    vec2   position_{.x = 0.f, .y = 0.f};
    extent window_size_;
    f32    zoom_{1.f};
};
}  // namespace rin