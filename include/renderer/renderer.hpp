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
class renderer final {
  public:
    [[nodiscard]] static auto create() noexcept -> std::expected<renderer, error> {
        auto shader_res = shader::create();
        if (not shader_res)
            return error::create(runtime_error, "Failed to create shader.", shader_res.error());

        return renderer{std::move(*shader_res)};
    }

    void use() noexcept { shader_.use(); }

    void draw(const shape auto& shape, const camera& camera) noexcept {
        const u32 points = shape.point_count();

        static thread_local auto vertices = std::vector<glm::vec2>{};
        vertices.clear();
        vertices.reserve(points + 1);

        vertices.emplace_back(0.0f, 0.0f);  // 中心点

        constexpr auto radius     = 0.5f;
        const auto     angle_step = (2.0f * std::numbers::pi_v<f32>) / static_cast<f32>(points);

        for (const auto i : std::views::indices(points)) {
            const f32 angle = angle_step * static_cast<f32>(i);
            vertices.emplace_back(radius * std::cos(angle), radius * std::sin(angle));
        }

        mesh_.update_vertices(vertices);

        const auto index_data = ebo_manager_.get_or_create(points);

        const auto model           = calc_transform(shape);
        const auto view_projection = camera.calc_view_position_mat();
        shader_.set_mat4(shader::u_Transform, view_projection * model);
        shader_.set_vec4(shader::u_Color, static_cast<glm::vec4>(shape.color()) / 255.f);

        mesh_.draw(index_data.ebo, index_data.index_count);
    }

  private:
    explicit renderer(shader shader_object) noexcept
        : shader_{std::move(shader_object)}, mesh_{mesh{setting::default_vbo_buffer}} {}

    ebo_manager ebo_manager_;
    shader      shader_;
    mesh        mesh_;
};
}  // namespace rin