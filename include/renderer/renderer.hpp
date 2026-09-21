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

#include "camera.hpp"
#include "error.hpp"
#include "renderer/mesh.hpp"
#include "renderer/shader.hpp"
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

        const auto rgbs = std::array<glm::vec3, 4>{
            glm::vec3{1.f, 0.f, 0.f},
            glm::vec3{0.f, 1.f, 0.f},
            glm::vec3{0.f, 0.f, 1.f},
            glm::vec3{1.f, 1.f, 0.f}
        };
        const auto indices = {0u, 1u, 2u, 2u, 3u, 0u};

        auto mesh_res = Mesh::create(points, rgbs, indices);

        if (not mesh_res)
            return Error::create(Error::runtime, "Failed to Create Quad Mesh", mesh_res.error());

        return Renderer{std::move(*shader_res), std::move(*mesh_res)};
    }

    void use() noexcept { shader_.use(); }

    void draw(const Quad& quad, const Camera& camera) noexcept {
        const auto model           = calc_transform(quad);
        const auto view_projection = camera.calc_view_position_mat();
        shader_.set_mat4(Shader::u_Transform, view_projection * model);

        shader_.set_vec4(Shader::u_Color, quad.color());
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