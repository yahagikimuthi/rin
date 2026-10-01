#pragma once

#include <compare>
#include <glm/ext/vector_float4.hpp>

#include "rin/types.hpp"

/**
 * @file color.hpp
 * @brief 8ビット整数によるRGBAカラー構造体
 */

namespace rin {
/**
 * @struct rgba
 * @brief 赤、緑、青、透過度（アルファ）の各要素を 8 ビット（u8）で保持するカラー構造体
 */
struct rgba final {
    /// 赤要素（0〜255）
    u8 r{0};
    /// 緑要素（0〜255）
    u8 g{0};
    /// 青要素（0〜255）
    u8 b{0};
    /// アルファ要素（0〜255、デフォルトは不透明: 255）
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
}  // namespace rin