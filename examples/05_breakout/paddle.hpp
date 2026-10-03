#pragma once

#include "rin/vec2.hpp"
#include "rin/vertex_vector.hpp"
#include "rin/window.hpp"

#include "examples/05_breakout/setting.hpp"

namespace gm {
class Paddle final {
  public:
    explicit Paddle() noexcept {
        vertices_.reserve(6);

        constexpr auto half_w = paddle_size.width / 2;
        constexpr auto half_h = paddle_size.height / 2;

        constexpr auto p1 = rin::vec2{.x = -half_w, .y = half_h};
        constexpr auto p2 = rin::vec2{.x = -half_w, .y = -half_h};
        constexpr auto p3 = rin::vec2{.x = half_w, .y = -half_h};
        constexpr auto p4 = rin::vec2{.x = half_w, .y = half_h};

        vertices_.emplace_back(p1, rin::white);
        vertices_.emplace_back(p2, rin::white);
        vertices_.emplace_back(p3, rin::white);

        vertices_.emplace_back(p3, rin::white);
        vertices_.emplace_back(p4, rin::white);
        vertices_.emplace_back(p1, rin::white);

        vertices_.position(paddle_center);
    }

    [[nodiscard]] auto x() const noexcept -> f32 { return position().x; }

    [[nodiscard]] auto position() const noexcept -> rin::vec2 { return vertices_.position(); }

    [[nodiscard]] auto top() const noexcept -> rin::vec2 {
        const auto pos = position();
        return rin::vec2{.x = pos.x, .y = pos.y + (paddle_size.height / 2)};
    }

    void x(const f32 value) noexcept { vertices_.position(value, position().y); }

    void draw(rin::window& window) noexcept { window.draw(vertices_); }

    void resize(const f32 size) noexcept { vertices_.scale(size, size); }

  private:
    rin::vertex_vector vertices_{rin::primitive_triangles};
};
}  // namespace gm