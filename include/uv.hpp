#pragma once

#include <compare>

#include "types.hpp"

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

    auto operator==(const uv&) const noexcept -> bool = default;
};
}  // namespace rin