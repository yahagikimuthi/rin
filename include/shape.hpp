#pragma once

#include <concepts>
#include <expected>
#include <utility>

#include "error.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "type.hpp"

namespace rin {
template <typename T>
concept Shape = requires(T shape) {
    { shape.draw() } noexcept -> std::same_as<void>;
};

class Quad final {
  public:
    [[nodiscard]] static auto create(const f32 width, const f32 height) noexcept
        -> std::expected<Quad, Error> {
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

        auto meshResult = Mesh::create(points, colors, indices);

        if (not meshResult)
            return Error::create(Error::logic, "Failed to Create Mesh", meshResult.error());
        return Quad{std::move(*meshResult)};
    }

    void draw(Shader& shader) const noexcept { mesh_.draw(); }

    Vector2f position{.x = 0, .y = 0};
    Vector2f size{.x = 0, .y = 0};
    f32      rotation{0.f};
    RGB      color{.r = 1.f, .g = 1.f, .b = 1.f};

  private:
    explicit Quad(Mesh mesh) noexcept : mesh_{std::move(mesh)} {}

    Mesh mesh_;
};
}  // namespace rin