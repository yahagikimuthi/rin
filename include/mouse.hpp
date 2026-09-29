#pragma once

#include "GLFW/glfw3.h"
#include "others/type.hpp"

namespace rin {
enum class mouse : u8 {
    left  = GLFW_MOUSE_BUTTON_LEFT,
    right = GLFW_MOUSE_BUTTON_RIGHT,
    wheel = GLFW_MOUSE_BUTTON_MIDDLE
};

inline constexpr auto mouse_left  = mouse::left;
inline constexpr auto mouse_right = mouse::right;
inline constexpr auto mouse_wheel = mouse::wheel;
}  // namespace rin