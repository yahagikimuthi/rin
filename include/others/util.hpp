#pragma once

#include <compare>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>

#include "others/type.hpp"

namespace rin {
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
        return rhs * lhs;
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
    u8 r{0};
    u8 g{0};
    u8 b{0};
    u8 a{255};

    [[nodiscard]] constexpr auto operator+() const noexcept -> rgba { return *this; }

    [[nodiscard]] constexpr auto operator==(const rgba&) const noexcept -> bool = default;

    [[nodiscard]] explicit constexpr operator glm::vec4() const noexcept {
        return glm::vec4{r, g, b, a};
    }
};

inline constexpr auto white   = rgba{.r = 255, .g = 255, .b = 255};
inline constexpr auto silver  = rgba{.r = 192, .g = 192, .b = 192};
inline constexpr auto gray    = rgba{.r = 128, .g = 128, .b = 128};
inline constexpr auto black   = rgba{.r = 0, .g = 0, .b = 0};
inline constexpr auto red     = rgba{.r = 255, .g = 0, .b = 0};
inline constexpr auto maroon  = rgba{.r = 128, .g = 0, .b = 0};
inline constexpr auto yellow  = rgba{.r = 255, .g = 255, .b = 0};
inline constexpr auto olive   = rgba{.r = 128, .g = 128, .b = 0};
inline constexpr auto lime    = rgba{.r = 0, .g = 255, .b = 0};
inline constexpr auto green   = rgba{.r = 0, .g = 128, .b = 0};
inline constexpr auto aqua    = rgba{.r = 0, .g = 255, .b = 255};
inline constexpr auto teal    = rgba{.r = 0, .g = 128, .b = 128};
inline constexpr auto blue    = rgba{.r = 0, .g = 0, .b = 255};
inline constexpr auto navy    = rgba{.r = 0, .g = 0, .b = 128};
inline constexpr auto fuchsia = rgba{.r = 255, .g = 0, .b = 255};
inline constexpr auto purple  = rgba{.r = 128, .g = 0, .b = 128};

struct uv final {
    f32 u{0.f};
    f32 v{0.f};

    auto operator==(const uv&) const noexcept -> bool = default;
};
}  // namespace rin