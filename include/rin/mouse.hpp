#pragma once

#include "rin/detail/graphics.hpp"
#include "rin/types.hpp"

/**
 * @file mouse.hpp
 * @brief マウスボタンの識別子および関連定数を定義するモジュール
 */

namespace rin {
/**
 * @enum mouse
 * @brief マウスのボタン種類を表す列挙体（基礎型: u8）
 */
enum class mouse : u8 {
    left  = GLFW_MOUSE_BUTTON_LEFT,
    right = GLFW_MOUSE_BUTTON_RIGHT,
    wheel = GLFW_MOUSE_BUTTON_MIDDLE
};

enum class scroll : u8 { up, down, count };

/// 以下はエイリアス
inline constexpr auto mouse_left  = mouse::left;
inline constexpr auto mouse_right = mouse::right;
inline constexpr auto mouse_wheel = mouse::wheel;
inline constexpr auto scroll_up   = scroll::up;
inline constexpr auto scroll_down = scroll::down;
}  // namespace rin