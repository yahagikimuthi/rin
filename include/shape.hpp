#pragma once

#include <concepts>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/vec2.hpp>
#include <utility>

#include "type.hpp"

namespace rin {
class BaseShape {};

template <typename T>
concept Shape = std::derived_from<T, BaseShape>;

class Quad final : public BaseShape {
  public:
    template <typename Self>
    [[nodiscard]] constexpr auto position(this Self&& self) noexcept -> auto&& {
        return std::forward<Self>(self).position_;
    }
    template <typename Self>
    [[nodiscard]] constexpr auto size(this Self&& self) noexcept -> auto&& {
        return std::forward<Self>(self).size_;
    }
    template <typename Self>
    [[nodiscard]] constexpr auto rotation_radian(this Self&& self) noexcept -> auto&& {
        return std::forward<Self>(self).rotation_radian_;
    }
    template <typename Self>
    [[nodiscard]] constexpr auto color(this Self&& self) noexcept -> auto&& {
        return std::forward<Self>(self).color_;
    }
    constexpr void position(const f32 x, const f32 y) noexcept { position_ = {x, y}; }
    constexpr void position(const glm::vec2 position) noexcept { position_ = position; }
    constexpr void size(const f32 width, const f32 height) noexcept { size_ = {width, height}; }
    constexpr void size(const glm::vec2 size) noexcept { size_ = size; }
    constexpr void color(const f32 r, const f32 g, const f32 b, const f32 alpha = 1.f) noexcept {
        color_ = {r, g, b, alpha};
    }
    constexpr void color(const glm::vec4& color) noexcept { color_ = color; }

  private:
    glm::vec4 color_{1.f, 1.f, 1.f, 1.f};
    glm::vec2 position_{0, 0};
    glm::vec2 size_{100.f, 100.f};
    f32       rotation_radian_{0.f};
};

[[nodiscard]] constexpr auto calc_transform(const Quad& quad) noexcept -> glm::mat4 {
    auto model = glm::mat4(1.0f);
    model      = glm::translate(model, glm::vec3(quad.position(), 0.0f));
    model      = glm::rotate(model, quad.rotation_radian(), glm::vec3(0.0f, 0.0f, 1.0f));
    model      = glm::scale(model, glm::vec3(quad.size(), 1.0f));
    return model;
}
}  // namespace rin