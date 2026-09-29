#pragma once

#include <cassert>
#include <expected>
#include <glm/ext/vector_float4.hpp>
#include <utility>

#include "others/error.hpp"
#include "others/setting.hpp"
#include "renderer/ebo_manager.hpp"
#include "renderer/mesh.hpp"
#include "renderer/shader.hpp"
#include "texture//text.hpp"
#include "texture/sprite.hpp"
#include "vertex.hpp"
#include "window/camera.hpp"

namespace rin {
class renderer final {
  public:
    [[nodiscard]] static auto try_make() noexcept -> std::expected<renderer, error> {
        auto shader_res = try_make_shader();
        if (not shader_res)
            return make_error(runtime_error, "Failed to create shader.", shader_res.error());

        return renderer{std::move(*shader_res)};
    }

    void use() noexcept { bind(shader_); }

    void draw(const vertex_vector& vertices, const camera& camera_obj) noexcept {
        const auto model = calc_transform_mat(vertices);
        const auto vp    = camera_obj.calc_view_position_mat();
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

    void draw(sprite& spr, const camera& camera) noexcept {
        const auto& tex = spr.setting_texture();

        bind(tex, 0);
        const auto& vertices = calc_vertices(spr);

        const auto model = calc_transform_mat(vertices);
        const auto vp    = camera.calc_view_position_mat();
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

    void draw(text& tex, const camera& camera_obj) noexcept {
        bind(tex, 0);

        const auto& vertices = calc_vertices(tex);

        const auto vp    = camera_obj.calc_view_position_mat();
        const auto model = calc_transform_mat(vertices);
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
        : shader_{std::move(shader_object)}, mesh_{make_mesh(default_vbo_buffer)} {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    ebo_manager ebo_manager_{make_ebo_manager()};
    shader      shader_;
    mesh        mesh_;
};

[[nodiscard]] inline auto try_make_renderer() noexcept -> std::expected<renderer, error> {
    return renderer::try_make();
}
}  // namespace rin