#pragma once

#include <concepts>

#include "mesh.hpp"
#include "type.hpp"
#include "util.hpp"

namespace rin {
template <typename T>
concept Shape = requires(T shape) {
    { shape.draw() } noexcept -> std::same_as<void>;
};

class Quad final {
  public:
    explicit Quad(const f32 width, const f32 height) noexcept : mesh_{createMesh(width, height)} {}
    explicit Quad(const Numeric auto width, const Numeric auto height) noexcept
        : Quad(static_cast<f32>(width), static_cast<f32>(height)) {}

    [[nodiscard]] static auto createMesh(const f32 width, const f32 height) noexcept -> Mesh {
        const auto hw = width * 0.5f;
        const auto hh = height * 0.5f;

        const auto points = std::array<Vector2f, 4>{
            Vector2f{.x = -hw, .y = -hh},
            Vector2f{.x = hw, .y = -hh},
            Vector2f{.x = hw, .y = hh},
            Vector2f{.x = -hw, .y = hh}
        };
        const auto colors = std::array<RGB, 4>{
            RGB{.r = 1.f, .g = 0.f, .b = 0.f},
            RGB{.r = 0.f, .g = 1.f, .b = 0.f},
            RGB{.r = 0.f, .g = 0.f, .b = 1.f},
            RGB{.r = 1.f, .g = 1.f, .b = 0.f}
        };
        const auto indices = {0u, 1u, 2u, 2u, 3u, 0u};

        return Mesh{points, colors, indices};
    }

    void draw() const noexcept { mesh_.draw(); }

    Mesh mesh_;
};
}  // namespace rin