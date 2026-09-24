#pragma once

#include <expected>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <optional>
#include <utility>

#include "camera.hpp"
#include "others/error.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "renderer/ebo_manager.hpp"
#include "renderer/mesh.hpp"
#include "renderer/shader.hpp"
#include "sprite.hpp"
#include "vertex.hpp"

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

    void draw(
        const vertex_vector& vec, const std::optional<const texture&> tex, const camera& camera_obj
    ) noexcept {
        shader_.set_mat4(shader::u_Transform, camera_obj.calc_view_position_mat());
        shader_.set_vec4(shader::u_Color, static_cast<glm::vec4>(white) / 255.f);
        shader_.set_bool(shader::u_UseTexture, false);

        if (tex) {
            tex->bind(0);
            shader_.set_int(shader::u_Texture, 0);
            shader_.set_bool(shader::u_UseTexture, true);
        } else {
            shader_.set_bool(shader::u_UseTexture, false);
        }

        mesh_.update_vertices(vec);
        const auto vertex_cnt = static_cast<u32>(vec.size());
        const auto index_data = ebo_manager_.get_or_create(vertex_cnt);
        mesh_.draw(index_data.ebo, index_data.index_count, vec.type());
    }

    void draw(const vertex_vector& vec, const camera& camera) noexcept {
        draw(vec, std::nullopt, camera);
    }

    void draw(sprite& sprite, const camera& camera) noexcept {
        draw(sprite.calc_vertices(), sprite.settle_texture(), camera);
    }

  private:
    explicit renderer(shader shader_object) noexcept
        : shader_{std::move(shader_object)}, mesh_{mesh{default_vbo_buffer}} {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    ebo_manager ebo_manager_;
    shader      shader_;
    mesh        mesh_;
};
}  // namespace rin