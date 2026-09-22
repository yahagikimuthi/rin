#pragma once

#include <expected>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <utility>

#include "camera.hpp"
#include "others/error.hpp"
#include "others/setting.hpp"
#include "renderer/ebo_manager.hpp"
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

        return Renderer{std::move(*shader_res), Mesh{setting::default_circle_segments}};
    }

    void use() noexcept { shader_.use(); }

    void draw(const IShape auto& shape, const Camera& camera) noexcept {
        const auto model = calc_transform(shape);
        // 汎用VAOは回転角を45度修正
        const auto offsetted_model =
            glm::rotate(model, glm::radians(45.f), glm::vec3(0.f, 0.f, 1.f));
        const auto view_projection = camera.calc_view_position_mat();
        shader_.set_mat4(Shader::u_Transform, view_projection * offsetted_model);

        shader_.set_vec4(Shader::u_Color, shape.color());
        const auto index_data = ebo_manager_.get_or_create(shape.point_count());
        mesh_.draw(index_data.ebo, index_data.index_count);
    }

  private:
    explicit Renderer(Shader shader, Mesh quad_mesh) noexcept
        : shader_{std::move(shader)}, mesh_{std::move(quad_mesh)} {}

    EBOManager ebo_manager_;
    Shader     shader_;
    Mesh       mesh_;
};
}  // namespace rin