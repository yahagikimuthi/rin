#pragma once

#include <concepts>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/vec2.hpp>
#include <type_traits>
#include <utility>

#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
template <typename T>
concept shape = requires(T shape) {
    { shape.position() } -> std::convertible_to<vec2>;
    { shape.size() } -> std::convertible_to<extent>;
    { shape.rotation_radian() } -> std::convertible_to<f32>;
    { shape.color() } -> std::convertible_to<rgba>;
    { shape.point_count() } -> std::convertible_to<u32>;
};

template <typename T>
struct is_shape : std::bool_constant<shape<T>> {};

template <typename T>
inline constexpr auto is_shape_v = is_shape<T>::value;

class polygon final {
  public:
    explicit constexpr polygon(u32 point_cnt) noexcept : point_cnt_{point_cnt} {}

    [[nodiscard]] constexpr auto position() const noexcept -> vec2 { return position_; }

    [[nodiscard]] constexpr auto size() const noexcept -> extent { return size_; }

    [[nodiscard]] constexpr auto rotation_radian() const noexcept -> f32 {
        return rotation_radian_;
    }

    [[nodiscard]] constexpr auto color() const noexcept -> rgba { return color_; }
    [[nodiscard]] constexpr auto point_count() const noexcept -> u32 { return point_cnt_; }

    constexpr void position(const f32 x, const f32 y) noexcept { position_ = {.x = x, .y = y}; }
    constexpr void position(const vec2 position) noexcept { position_ = position; }
    constexpr void size(const f32 width, const f32 height) noexcept {
        size_ = {.width = width, .height = height};
    }
    constexpr void size(const extent size) noexcept { size_ = size; }
    constexpr void color(const u8 r, const u8 g, const u8 b, const u8 a = 1.f) noexcept {
        color_ = {.r = r, .g = g, .b = b, .a = a};
    }
    constexpr void color(const rgba& color) noexcept { color_ = color; }
    constexpr void rotation_radian(const f32 radian) noexcept { rotation_radian_ = radian; }

  private:
    rgba   color_{};
    vec2   position_{};
    extent size_{.width = 100.f, .height = 100.f};
    f32    rotation_radian_{0.f};
    u32    point_cnt_;
};

[[nodiscard]] constexpr auto calc_transform(const shape auto& quad) noexcept -> glm::mat4 {
    auto model = glm::mat4(1.0f);
    model      = glm::translate(model, glm::vec3(static_cast<glm::vec2>(quad.position()), 0.0f));
    model      = glm::rotate(model, quad.rotation_radian(), glm::vec3(0.0f, 0.0f, 1.0f));
    model      = glm::scale(model, glm::vec3(static_cast<glm::vec2>(quad.size()), 1.0f));
    return model;
}
}  // namespace rin