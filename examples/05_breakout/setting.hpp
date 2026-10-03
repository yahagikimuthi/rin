#pragma once

#include <array>

#include "rin/color.hpp"
#include "rin/extent.hpp"
#include "rin/types.hpp"
#include "rin/vec2.hpp"

namespace gm {
using rin::f32;
using rin::i32;
using rin::u32;
using rin::u8;

inline constexpr auto window_size = rin::extent{.width = 800, .height = 600};
inline constexpr auto paddle_size =
    rin::extent{.width = window_size.width / 8, .height = window_size.height / 30};
inline constexpr auto paddle_center =
    rin::vec2{.x = window_size.width / 2, .y = window_size.height / 30};
inline constexpr auto ball_radius = 10.f;
inline constexpr auto ball_center = []() constexpr noexcept -> rin::vec2 {
    constexpr auto paddle_top = paddle_center.y + (paddle_size.height / 2);
    return {.x = paddle_center.x, .y = paddle_top + ball_radius};
}();
inline constexpr auto init_ball_speed               = 500.f;
inline constexpr auto block_row                     = 10;
inline constexpr auto block_col                     = 6;
inline constexpr auto block_margin                  = 5.f;
inline constexpr auto block_side_blank              = 40.f;
inline constexpr auto block_occupancy_window_height = 0.4f;
inline constexpr auto block_size                    = []() constexpr noexcept -> rin::extent {
    constexpr auto width = []() constexpr noexcept -> f32 {
        constexpr auto total_margin    = (block_row - 1) * block_margin;
        constexpr auto available_width = window_size.width - total_margin - (block_side_blank * 2);
        return available_width / block_row;
    }();

    constexpr auto height = []() constexpr noexcept -> f32 {
        constexpr auto total_margin = (block_col - 1) * block_margin;
        constexpr auto available_height =
            (window_size.height * block_occupancy_window_height) - total_margin - block_side_blank;
        return available_height / block_col;
    }();

    return rin::extent{.width = width, .height = height};
}();
inline constexpr auto block_col_colors = std::array<rin::rgba, block_col>{
    rin::red, rin::yellow, rin::green, rin::aqua, rin::blue, rin::purple
};

enum class GameState : u8 { waiting, playing, clear, over };
}  // namespace gm