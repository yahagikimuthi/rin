#pragma once

#include <array>
#include <expected>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

#include "error.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "shape.hpp"

namespace rin {
class Renderer final {
  public:
    [[nodiscard]] static auto create() noexcept -> std::expected<Renderer, Error> {
        auto shader_res = Shader::create();
        if (not shader_res)
            return Error::create(Error::runtime, "Failed to Create Shader", shader_res.error());

        const auto points = std::array<glm::vec2, 4>{
            glm::vec2{-0.5f, -0.5f},
            glm::vec2{0.5f, -0.5f},
            glm::vec2{0.5f, 0.5f},
            glm::vec2{-0.5f, 0.5f}
        };

        const auto colors = std::array<RGB, 4>{
            RGB{.r = 1.f, .g = 0.f, .b = 0.f},
            RGB{.r = 0.f, .g = 1.f, .b = 0.f},
            RGB{.r = 0.f, .g = 0.f, .b = 1.f},
            RGB{.r = 1.f, .g = 1.f, .b = 0.f}
        };
        const auto indices = {0u, 1u, 2u, 2u, 3u, 0u};

        auto mesh_res = Mesh::create(points, colors, indices);

        if (not mesh_res)
            return Error::create(Error::runtime, "Failed to Create Quad Mesh", mesh_res.error());

        return Renderer{std::move(*shader_res), std::move(*mesh_res)};
    }

    void use() noexcept { shader_.use(); }

    void draw([[maybe_unused]] const Quad& quad) noexcept {
        const auto model = calc_transform(quad);
        shader_.set_mat4(Shader::u_Transform, model);

        const auto color = quad.color;
        shader_.set_vec4(Shader::u_Color, {color.r, color.g, color.b, 1.f});
        quad_mesh_.draw();
    }

  private:
    explicit Renderer(Shader shader, Mesh quad_mesh) noexcept
        : shader_{std::move(shader)}, quad_mesh_{std::move(quad_mesh)} {}

    Shader shader_;
    Mesh   quad_mesh_;

    friend class Engine;
};
}  // namespace rin