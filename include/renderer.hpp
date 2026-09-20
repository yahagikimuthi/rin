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

    void draw([[maybe_unused]] const Quad& quad) noexcept {
        //        auto model = glm::mat4(1.f);
        //        model      = glm::translate(model, glm::vec3(quad.position, 0.f));
        //        if (quad.rotation_radius != 0.f)
        //            model = glm::rotate(model, quad.rotation_radius, glm::vec3(0.f, 0.f, 1.f));
        //
        //        model = glm::scale(model, glm::vec3(quad.size, 1.f));
        //        shader_.set_mat4(Shader::u_Transform, model);
        quad_mesh_.draw();
    }

    void use() noexcept { shader_.use(); }

  private:
    explicit Renderer(Shader shader, Mesh quad_mesh) noexcept
        : shader_{std::move(shader)}, quad_mesh_{std::move(quad_mesh)} {}

    Shader shader_;
    Mesh   quad_mesh_;
};
}  // namespace rin