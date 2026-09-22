#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <type_traits>

#include <variant>
#include "others/type.hpp"

namespace rin {
template <typename T, typename Variant>
struct is_variant_member : std::false_type {};

template <typename T, typename... Ts>
struct is_variant_member<T, std::variant<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> or ...)> {};

template <typename T, typename Variant>
concept variant_member = is_variant_member<T, Variant>::value;

template <typename T, typename Variant>
static constexpr auto is_variant_member_v = is_variant_member<T, Variant>::value;

struct vec2 final {
    f32 x{};
    f32 y{};

    [[nodiscard]] explicit constexpr operator glm::vec2() const noexcept { return glm::vec2{x, y}; }
};

struct extend final {
    f32 width{};
    f32 height{};

    [[nodiscard]] explicit constexpr operator glm::vec2() const noexcept {
        return glm::vec2{width, height};
    }
};

struct rgba final {
    f32 r{0.f};
    f32 g{0.f};
    f32 b{0.f};
    f32 a{255.f};

    [[nodiscard]] explicit constexpr operator glm::vec4() const noexcept {
        return glm::vec4{r, g, b, a};
    }
};

namespace color {
static constexpr auto white = rgba{.r = 255.f, .g = 255.f, .b = 255.f, .a = 255.f};
static constexpr auto red   = rgba{.r = 255.f, .g = 0.f, .b = 0.f, .a = 255.f};
}  // namespace color
}  // namespace rin