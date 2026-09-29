#pragma once

#include <cmath>
#include <glm/ext/vector_float2.hpp>

#include "others/type.hpp"

namespace rin {
struct vec2 final {
    f32 x{};
    f32 y{};

    [[nodiscard]] constexpr auto abs() const noexcept -> f32 { return std::hypot(x, y); }

    [[nodiscard]] constexpr auto sum_square() const noexcept -> f32 { return (x * x) + (y * y); }

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
}  // namespace rin