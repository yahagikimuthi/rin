#pragma once

#include "rin/types.hpp"

/**
 * @file uv.hpp
 * @brief テクスチャ座標（UV座標）を表現する構造体モジュール
 */

namespace rin {
/**
 * @struct uv
 * @brief テクスチャの正規化座標（U, V）を保持する構造体
 */
struct uv final {
    /// U座標（水平方向、通常 0.0 〜 1.0）
    f32 u{};

    /// V座標（垂直方向、通常 0.0 〜 1.0）
    f32 v{};

    constexpr auto operator+=(const uv other) noexcept -> uv& {
        u += other.u;
        v += other.v;
        return *this;
    }
    constexpr auto operator-=(const uv other) noexcept -> uv& {
        u -= other.u;
        v -= other.v;
        return *this;
    }
    constexpr auto operator*=(const f32 other) noexcept -> uv& {
        u *= other;
        v *= other;
        return *this;
    }
    constexpr auto operator/=(const f32 other) noexcept -> uv& {
        u /= other;
        v /= other;
        return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(uv lhs, uv rhs) noexcept -> uv {
        lhs += rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator-(uv lhs, uv rhs) noexcept -> uv {
        lhs -= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(uv lhs, f32 rhs) noexcept -> uv {
        lhs *= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(f32 lhs, uv rhs) noexcept -> uv {
        return rhs * lhs;
    }
    [[nodiscard]] friend constexpr auto operator/(uv lhs, f32 rhs) noexcept -> uv {
        lhs /= rhs;
        return lhs;
    }

    [[nodiscard]] constexpr auto operator+() const noexcept -> uv { return *this; }
    [[nodiscard]] constexpr auto operator-() const noexcept -> uv { return uv{.u = -u, .v = -v}; }

    [[nodiscard]] constexpr auto operator==(const uv&) const noexcept -> bool = default;
};
}  // namespace rin