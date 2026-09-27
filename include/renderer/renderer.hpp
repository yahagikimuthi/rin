#pragma once

#include <cassert>
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

#include "others/error.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
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

    void use() noexcept { shader_.use(); }

    void draw(const vertex_vector& vec, const camera& camera) noexcept {
        draw(vec, std::nullopt, camera);
    }

    void draw(sprite& spr, const camera& camera) noexcept {
        const auto tex = spr.setting_texture();
        if (not tex) return;

        tex->bind(0);
        static auto tmp_vertex_buff = make_vertex_vector(primitive_triangles);
        tmp_vertex_buff.clear();
        spr.append_to(tmp_vertex_buff);

        const auto model = spr.calc_transfrom_mat();
        const auto vp    = camera.calc_view_position_mat();
        const auto mvp   = vp * model;

        shader_.set_mat4(shader::u_Transform, mvp);
        shader_.set_vec4(shader::u_Color, static_cast<glm::vec4>(white) / 255.f);
        shader_.set_bool(shader::u_UseTexture, true);

        mesh_.update_vertices(tmp_vertex_buff);
        assert(tmp_vertex_buff.type() == primitive_triangles);

        const auto vertex_cnt = static_cast<u32>(tmp_vertex_buff.size());
        const auto index_data = ebo_manager_.get_or_create(vertex_cnt);
        mesh_.draw_elements(index_data.ebo, index_data.index_count, primitive_triangles);
    }

    void draw(text& tex, const camera& camera_obj) noexcept {
        if (not tex.font_) return;

        const auto& vertices = tex.calc_vertices();

        const auto transform = glm::translate(
            camera_obj.calc_view_position_mat(),
            glm::vec3{vertices.position().x, vertices.position().y, 0.f}
        );
        shader_.set_mat4(shader::u_Transform, transform);
        shader_.set_vec4(shader::u_Color, static_cast<glm::vec4>(white) / 255.f);

        tex.font_->setting_texture().bind(0);
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

    //? 過度に関数を共通化するのは読みにくい可能性がある
    void draw(
        const vertex_vector&                vec,
        const std::optional<const texture&> texture_ref,
        const camera&                       camera_obj
    ) noexcept {
        const auto transform = glm::translate(
            camera_obj.calc_view_position_mat(), glm::vec3{vec.position().x, vec.position().y, 0.f}
        );
        shader_.set_mat4(shader::u_Transform, transform);
        shader_.set_vec4(shader::u_Color, static_cast<glm::vec4>(white) / 255.f);

        if (texture_ref) {
            texture_ref->bind(0);
            shader_.set_int(shader::u_Texture, 0);
            shader_.set_bool(shader::u_UseTexture, true);
        } else {
            shader_.set_bool(shader::u_UseTexture, false);
        }

        mesh_.update_vertices(vec);
        if (vec.type() == primitive_type::triangles) {
            const auto vertex_cnt = static_cast<u32>(vec.size());
            const auto index_data = ebo_manager_.get_or_create(vertex_cnt);
            mesh_.draw_elements(index_data.ebo, index_data.index_count, vec.type());
            return;
        }

        mesh_.draw_arrays(static_cast<GLsizei>(vec.size()), vec.type());
    }

    ebo_manager ebo_manager_{make_ebo_manager()};
    shader      shader_;
    mesh        mesh_;
};

[[nodiscard]] inline auto try_make_renderer() noexcept -> std::expected<renderer, error> {
    return renderer::try_make();
}
}  // namespace rin