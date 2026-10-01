#pragma once

#include <cmath>
#include <glm/ext/vector_float2.hpp>

#include "rin/types.hpp"

/**
 * @file vec2.hpp
 * @brief 2次元ベクトルを表現する構造体および関連演算子の定義モジュール
 */

namespace rin {
/**
 * @struct vec2
 * @brief 2次元ベクトル（X, Y要素）を保持する構造体
 */
struct vec2 final {
    /// X成分
    f32 x{};

    /// Y成分
    f32 y{};

    /**
     * @brief ベクトルの長さ（絶対値 / ノルム）を計算します。
     * @return f32 ベクトルの長さ
     */
    [[nodiscard]] constexpr auto abs() const noexcept -> f32 { return std::hypot(x, y); }

    /**
     * @brief ベクトルの各成分の二乗和（長さの2乗）を計算します。
     * @return f32 各成分の二乗和
     */
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

    /**
     * @brief glm::vec2 型への明示的キャスト演算子
     * @return glm::vec2 変換後の glm::vec2 オブジェクト
     */
    [[nodiscard]] explicit constexpr operator glm::vec2() const noexcept { return glm::vec2{x, y}; }
};
}  // namespace rin