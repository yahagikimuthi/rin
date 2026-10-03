#pragma once

#include <cassert>
#include <expected>
#include <glm/ext/vector_float4.hpp>
#include <utility>

#include "rin/detail/camera.hpp"
#include "rin/detail/ebo_manager.hpp"
#include "rin/detail/mesh.hpp"
#include "rin/detail/shader.hpp"
#include "rin/error.hpp"
#include "rin/extent.hpp"
#include "rin/sprite.hpp"
#include "rin/text.hpp"
#include "rin/vertex_vector.hpp"

namespace rin::detail {
class renderer final {
  public:
    [[nodiscard]] static auto try_make() noexcept -> std::expected<renderer, error> {
        auto shader_res = shader::try_make();
        if (not shader_res)
            return make_error(runtime_error, "Failed to create shader.", shader_res.error());

        return renderer{std::move(*shader_res)};
    }

    void use() noexcept { shader_.bind(); }

    void draw(
        const vertex_vector& vertices, const camera& camera_obj, const extent& virtual_window_size
    ) noexcept {
        const auto model = vertices.calc_transform_mat();
        const auto vp    = camera_obj.calc_view_position_mat(virtual_window_size);
        const auto mvp   = vp * model;
        shader_.set_mat4(shader::u_Transform, mvp);
        shader_.set_vec4(shader::u_Color, static_cast<glm::vec4>(vertices.color()) / 255.f);
        shader_.set_bool(shader::u_UseTexture, false);

        mesh_.update_vertices(vertices);
        if (vertices.type() != primitive_triangles) {
            mesh_.draw_arrays(static_cast<GLsizei>(vertices.size()), vertices.type());
            return;
        }

        const auto vertex_cnt = static_cast<u32>(vertices.size());
        const auto index_data = ebo_manager_.get_or_create(vertex_cnt);
        mesh_.draw_elements(index_data.ebo, index_data.index_count, vertices.type());
    }

    void draw(sprite& spr, const camera& camera, const extent& virtual_window_size) noexcept {
        const auto& tex = spr.setting_texture();

        tex.bind(0);
        const auto& vertices = spr.calc_vertices();

        const auto model = vertices.calc_transform_mat();
        const auto vp    = camera.calc_view_position_mat(virtual_window_size);
        const auto mvp   = vp * model;

        shader_.set_mat4(shader::u_Transform, mvp);
        shader_.set_vec4(shader::u_Color, static_cast<glm::vec4>(vertices.color()) / 255.f);
        shader_.set_bool(shader::u_UseTexture, true);

        mesh_.update_vertices(vertices);
        assert(vertices.type() == primitive_triangles);

        const auto vertex_cnt = static_cast<u32>(vertices.size());
        const auto index_data = ebo_manager_.get_or_create(vertex_cnt);
        mesh_.draw_elements(index_data.ebo, index_data.index_count, primitive_triangles);
    }

    void draw(text& tex, const camera& camera_obj, const extent& virtual_window_size) noexcept {
        tex.bind(0);

        const auto& vertices = tex.calc_vertices();

        const auto vp    = camera_obj.calc_view_position_mat(virtual_window_size);
        const auto model = vertices.calc_transform_mat();
        const auto mvp   = vp * model;

        shader_.set_mat4(shader::u_Transform, mvp);
        shader_.set_vec4(shader::u_Color, static_cast<glm::vec4>(vertices.color()) / 255.f);

        shader_.set_int(shader::u_Texture, 0);
        shader_.set_bool(shader::u_UseTexture, true);

        mesh_.update_vertices(vertices);
        mesh_.draw_arrays(static_cast<GLsizei>(vertices.size()), vertices.type());
    }

  private:
    explicit renderer(shader shader_object) noexcept
        : shader_{std::move(shader_object)}, mesh_{1024} {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    ebo_manager ebo_manager_;
    shader      shader_;
    mesh        mesh_;
};
}  // namespace rin::detail