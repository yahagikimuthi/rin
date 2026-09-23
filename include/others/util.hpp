#pragma once

#include <cmath>
#include <compare>
#include <concepts>
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <type_traits>
#include <variant>

#include "others/type.hpp"

namespace rin {
template <typename T, typename Variant>
struct is_variant_member final : std::false_type {};

template <typename T, typename... Ts>
struct is_variant_member<T, std::variant<Ts...>> final
    : std::bool_constant<(std::is_same_v<T, Ts> or ...)> {};

template <typename T, typename Variant>
concept variant_member = is_variant_member<T, Variant>::value;

template <typename T, typename Variant>
inline constexpr auto is_variant_member_v = is_variant_member<T, Variant>::value;

template <typename... Ts>
struct overloaded final : public Ts... {
    using Ts::operator()...;
};

template <std::invocable<> F>
class scope_exit final {
  public:
    [[nodiscard]] explicit scope_exit(F func) noexcept : exit_func_{func} {};
    scope_exit(const scope_exit&) noexcept                   = delete;
    auto operator=(const scope_exit) noexcept -> scope_exit& = delete;
    scope_exit(scope_exit&&) noexcept                        = delete;
    auto operator=(scope_exit&&) noexcept -> scope_exit&&    = delete;
    ~scope_exit() noexcept { std::invoke(exit_func_); }

  private:
    F exit_func_;
};

struct vec2 final {
    f32 x{};
    f32 y{};

    [[nodiscard]] constexpr auto hypot() const noexcept -> f32 { return std::hypot(x, y); }

    constexpr auto operator+=(const vec2 other) noexcept -> vec2& {
        x += other.x;
        y += other.y;
        return *this;
    }
    constexpr auto operator-=(const vec2 other) noexcept -> vec2& {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    constexpr auto operator*=(const f32 value) noexcept -> vec2& {
        x *= value;
        y *= value;
        return *this;
    }
    constexpr auto operator/=(const f32 value) noexcept -> vec2& {
        x /= value;
        y /= value;
        return *this;
    }

    [[nodiscard]] constexpr auto operator+() const noexcept -> vec2 { return *this; }
    [[nodiscard]] constexpr auto operator-() const noexcept -> vec2 {
        return vec2{.x = -x, .y = -y};
    }

    constexpr auto operator==(const vec2&) const noexcept -> bool = default;

    [[nodiscard]] friend constexpr auto operator+(vec2 lhs, const vec2& rhs) noexcept -> vec2 {
        lhs += rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator-(vec2 lhs, const vec2& rhs) noexcept -> vec2 {
        lhs -= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(vec2 lhs, f32 rhs) noexcept -> vec2 {
        lhs *= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(f32 lhs, vec2 rhs) noexcept -> vec2 {
        return rhs * lhs;
    }
    [[nodiscard]] friend constexpr auto operator/(vec2 lhs, f32 rhs) noexcept -> vec2 {
        lhs /= rhs;
        return lhs;
    }

    [[nodiscard]] explicit constexpr operator glm::vec2() const noexcept { return glm::vec2{x, y}; }
};

struct extent final {
    f32 width{};
    f32 height{};

    [[nodiscard]] constexpr auto square() const noexcept -> f32 { return width * height; }

    constexpr auto operator+=(const extent other) noexcept -> extent& {
        width += other.width;
        height += other.height;
        return *this;
    }
    constexpr auto operator-=(const extent other) noexcept -> extent& {
        width -= other.width;
        height -= other.height;
        return *this;
    }
    constexpr auto operator*=(const f32 value) noexcept -> extent& {
        width *= value;
        height *= value;
        return *this;
    }
    constexpr auto operator/=(const f32 value) noexcept -> extent& {
        width /= value;
        height /= value;
        return *this;
    }

    [[nodiscard]] constexpr auto operator==(const extent&) const noexcept -> bool = default;
    [[nodiscard]] constexpr auto operator<=>(const extent other) const noexcept -> auto {
        return square() <=> other.square();
    }

    [[nodiscard]] constexpr auto operator+() const noexcept -> extent { return *this; }
    [[nodiscard]] constexpr auto operator-() const noexcept -> extent {
        return extent{.width = -width, .height = -height};
    }

    [[nodiscard]] friend constexpr auto operator+(extent lhs, const extent& rhs) noexcept
        -> extent {
        lhs += rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator-(extent lhs, const extent& rhs) noexcept
        -> extent {
        lhs -= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(extent lhs, const f32 rhs) noexcept -> extent {
        lhs *= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(const f32 lhs, extent rhs) noexcept -> extent {
        return rhs *= lhs;
    }
    [[nodiscard]] friend constexpr auto operator/(extent lhs, const f32 rhs) noexcept -> extent {
        lhs /= rhs;
        return lhs;
    }

    [[nodiscard]] explicit constexpr operator glm::vec2() const noexcept {
        return glm::vec2{width, height};
    }
};

struct rgba final {
    f32 r{0.f};
    f32 g{0.f};
    f32 b{0.f};
    f32 a{255.f};

    [[nodiscard]] constexpr auto operator+() const noexcept -> rgba { return *this; }

    [[nodiscard]] constexpr auto operator==(const rgba&) const noexcept -> bool = default;

    [[nodiscard]] explicit constexpr operator glm::vec4() const noexcept {
        return glm::vec4{r, g, b, a};
    }
};

inline constexpr auto white   = rgba{.r = 255.f, .g = 255.f, .b = 255.f};
inline constexpr auto silver  = rgba{.r = 192.f, .g = 192.f, .b = 192.f};
inline constexpr auto gray    = rgba{.r = 128.f, .g = 128.f, .b = 128.f};
inline constexpr auto black   = rgba{.r = 0.f, .g = 0.f, .b = 0.f};
inline constexpr auto red     = rgba{.r = 255.f, .g = 0.f, .b = 0.f};
inline constexpr auto maroon  = rgba{.r = 128.f, .g = 0.f, .b = 0.f};
inline constexpr auto yellow  = rgba{.r = 255.f, .g = 255.f, .b = 0.f};
inline constexpr auto olive   = rgba{.r = 128.f, .g = 128.f, .b = 0.f};
inline constexpr auto lime    = rgba{.r = 0.f, .g = 255.f, .b = 0.f};
inline constexpr auto green   = rgba{.r = 0.f, .g = 128.f, .b = 0.f};
inline constexpr auto aqua    = rgba{.r = 0.f, .g = 255.f, .b = 255.f};
inline constexpr auto teal    = rgba{.r = 0.f, .g = 128.f, .b = 128.f};
inline constexpr auto blue    = rgba{.r = 0.f, .g = 0.f, .b = 255.f};
inline constexpr auto navy    = rgba{.r = 0.f, .g = 0.f, .b = 128.f};
inline constexpr auto fuchsia = rgba{.r = 255.f, .g = 0.f, .a = 255.f};
inline constexpr auto purple  = rgba{.r = 128.f, .g = 0.f, .b = 128.f};
}  // namespace rin