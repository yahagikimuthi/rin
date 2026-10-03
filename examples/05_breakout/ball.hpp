#pragma once

#include <cmath>
#include <ranges>

#include "rin/color.hpp"
#include "rin/vec2.hpp"
#include "rin/vertex_vector.hpp"
#include "rin/window.hpp"

#include "examples/05_breakout/setting.hpp"

namespace gm {
class Ball final {
  public:
    explicit Ball() noexcept {
        constexpr auto segments   = 64;
        constexpr auto angle_step = (2.f * std::numbers::pi_v<f32>) / static_cast<f32>(segments);

        vertices_.reserve(192);
        for (const auto i : std::views::indices(segments)) {
            const auto a1 = static_cast<f32>(i) * angle_step;
            const auto a2 = static_cast<f32>(i + 1) * angle_step;
            const auto p1 =
                rin::vec2{.x = std::cos(a1) * ball_radius, .y = std::sin(a1) * ball_radius};
            const auto p2 =
                rin::vec2{.x = std::cos(a2) * ball_radius, .y = std::sin(a2) * ball_radius};

            vertices_.emplace_back(rin::vec2{}, rin::white);
            vertices_.emplace_back(p1, rin::white);
            vertices_.emplace_back(p2, rin::white);

            vertices_.position(ball_center);
        }
    }

    [[nodiscard]] auto position() const noexcept -> rin::vec2 { return vertices_.position(); }

    [[nodiscard]] auto velocity() const noexcept -> rin::vec2 { return velocity_; }

    void position(const rin::vec2 pos) noexcept { vertices_.position(pos); }
    void position(const f32 x, const f32 y) noexcept { vertices_.position(x, y); }

    void velocity(const rin::vec2 v) noexcept { velocity_ = v; }
    void velocity(const f32 x, const f32 y) noexcept { velocity_ = {.x = x, .y = y}; }

    void draw(rin::window& window) noexcept { window.draw(vertices_); }

    void move(const f32 delta_time) noexcept {
        const auto after_pos = position() + (velocity_ * delta_time);
        position(after_pos);
    }

  private:
    rin::vertex_vector vertices_{rin::primitive_triangles};
    rin::vec2          velocity_{};
};
}  // namespace gm