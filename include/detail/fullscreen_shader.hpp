#pragma once

#include <expected>
#include <optional>
#include <utility>

#include "detail/graphics.hpp"
#include "error.hpp"

namespace rin::detail {
class fullscreen_shader final {
    static constexpr auto* vert = R"(
    #version 450 core

    layout (location = 0) out vec2 v_uv;

    void main() {
        // gl_VertexID (0, 1, 2) から画面 (-1.0 ～ 1.0) を完全に覆う1つの巨大な三角形を生成
        // 頂点 0: (-1.0, -1.0), UV: (0.0, 0.0)
        // 頂点 1: ( 3.0, -1.0), UV: (2.0, 0.0)
        // 頂点 2: (-1.0,  3.0), UV: (0.0, 2.0)
        float x = -1.0 + float((gl_VertexID & 1) << 2);
        float y = -1.0 + float((gl_VertexID & 2) << 1);

        v_uv = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5);
        gl_Position = vec4(x, y, 0.0, 1.0);
    }
    )";

    static constexpr auto* frag = R"(
    #version 450 core

    layout (location = 0) in vec2 v_uv;
    layout (binding = 0) uniform sampler2D u_fbo_texture;

    layout (location = 0) out vec4 frag_color;

    void main() {
        frag_color = texture(u_fbo_texture, v_uv);
    }
    )";

  public:
    [[nodiscard]] static auto try_make() noexcept -> std::expected<fullscreen_shader, error> {
        const auto vertex_shader = compile_shader(GL_VERTEX_SHADER, vert);
        if (not vertex_shader) return make_error(logic_error, "Failed to compile vertex shader.");
        const auto frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag);
        if (not frag_shader) return make_error(logic_error, "Failed to compile fragment shader.");

        const auto program = glCreateProgram();
        glAttachShader(program, *vertex_shader);
        glAttachShader(program, *frag_shader);
        glLinkProgram(program);
        if (GL_LINK_STATUS == GL_FALSE) return make_error(logic_error, "Failed to link.");
        glDeleteShader(*vertex_shader);
        glDeleteShader(*frag_shader);

        return fullscreen_shader{program};
    }

    fullscreen_shader(const fullscreen_shader&) noexcept                    = delete;
    auto operator=(const fullscreen_shader&) noexcept -> fullscreen_shader& = delete;

    fullscreen_shader(fullscreen_shader&& other) noexcept
        : program_id_{std::exchange(other.program_id_, 0)} {}
    auto operator=(fullscreen_shader&& other) noexcept -> fullscreen_shader& {
        if (this == &other) return *this;
        destroy();
        program_id_ = std::exchange(other.program_id_, 0);
        return *this;
    }

    ~fullscreen_shader() noexcept { destroy(); }

  private:
    explicit fullscreen_shader(const GLuint program_id) noexcept : program_id_{program_id} {}

    [[nodiscard]] static auto compile_shader(GLuint type, std::string_view source) noexcept
        -> std::optional<GLuint> {
        auto        shader = glCreateShader(type);
        const auto* src    = source.data();
        const auto  len    = static_cast<GLint>(source.size());
        glShaderSource(shader, 1, &src, &len);
        glCompileShader(shader);

        if (GL_COMPILE_STATUS == GL_FALSE) return std::nullopt;

        return shader;
    }

    void destroy() noexcept {
        if (program_id_ != 0) {
            glDeleteProgram(program_id_);
            program_id_ = 0;
        }
    }

    GLuint program_id_{};
};
}  // namespace rin::detail