#pragma once

#include <concepts>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/vec2.hpp>
#include <type_traits>
#include <utility>

#include "others/setting.hpp"
#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
template <typename T>
concept shape = requires(T shape) {
    { shape.position() } -> std::convertible_to<vec2>;
    { shape.size() } -> std::convertible_to<extend>;
    { shape.rotation_radian() } -> std::convertible_to<f32>;
    { shape.color() } -> std::convertible_to<rgba>;
    { shape.point_count() } -> std::convertible_to<u32>;
};

template <typename T>
struct is_shape : std::bool_constant<shape<T>> {};

template <typename T>
static constexpr auto is_shape_v = is_shape<T>::value;

class polygon {
  public:
    explicit constexpr polygon(u32 point_cnt) noexcept : point_cnt_{point_cnt} {}
    constexpr ~polygon() noexcept = default;

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
    [[nodiscard]] constexpr auto point_count() const noexcept -> u32 { return point_cnt_; }

    constexpr void position(const f32 x, const f32 y) noexcept { position_ = {.x = x, .y = y}; }
    constexpr void position(const vec2 position) noexcept { position_ = position; }
    constexpr void size(const f32 width, const f32 height) noexcept {
        size_ = {.width = width, .height = height};
    }
    constexpr void size(const extend size) noexcept { size_ = size; }
    constexpr void color(const f32 r, const f32 g, const f32 b, const f32 a = 1.f) noexcept {
        color_ = {.r = r, .g = g, .b = b, .a = a};
    }
    constexpr void color(const rgba& color) noexcept { color_ = color; }
    constexpr void rotation_radian(f32 rotation) noexcept { rotation_radian_ = rotation; }

  protected:
    constexpr polygon(const polygon&) noexcept                    = default;
    constexpr auto operator=(const polygon&) noexcept -> polygon& = default;
    constexpr polygon(polygon&&) noexcept                         = default;
    constexpr auto operator=(polygon&&) noexcept -> polygon&      = default;

  private:
    rgba   color_{};
    vec2   position_{};
    extend size_{.width = 100.f, .height = 100.f};
    f32    rotation_radian_{0.f};
    u32    point_cnt_;
};

class circle final : public polygon {
  public:
    explicit constexpr circle(const u32 points) : polygon(points) {}
    explicit constexpr circle() noexcept : polygon(setting::default_circle_segments) {}
};

class triangle final : public polygon {
  public:
    explicit constexpr triangle() noexcept : polygon(3uz) {}
};
class quad final : public polygon {
  public:
    explicit constexpr quad() noexcept : polygon(4uz) {}
};

[[nodiscard]] constexpr auto calc_transform(shape auto& quad) noexcept -> glm::mat4 {
    auto model = glm::mat4(1.0f);
    model      = glm::translate(model, glm::vec3(static_cast<glm::vec2>(quad.position()), 0.0f));
    model      = glm::rotate(model, quad.rotation_radian(), glm::vec3(0.0f, 0.0f, 1.0f));
    model      = glm::scale(model, glm::vec3(static_cast<glm::vec2>(quad.size()), 1.0f));
    return model;
}
}  // namespace rin