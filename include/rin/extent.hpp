#pragma once

#include <compare>
#include <glm/ext/vector_float2.hpp>

#include "rin/types.hpp"

/**
 * @file extent.hpp
 * @brief 2Dのサイズ（幅と高さ）を表す構造体
 */

namespace rin {
/**
 * @struct extent
 * @brief 幅（width）と高さ（height）を保持する 2D サイズ構造体
 */
struct extent final {
    /// 幅
    f32 width{};
    /// 高さ
    f32 height{};

    /**
     * @brief 面積（幅 × 高さ）を算出します。
     *
     * @return f32 算出された面積
     */
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
}  // namespace rin