#pragma once

#include <expected>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <string_view>
#include <utility>

#include "glad/glad.h"

#include "others/error.hpp"
#include "others/type.hpp"

namespace rin {
constexpr const char* vertex_shader_source = R"(
    #version 450 core
    
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    layout (location = 2) in vec4 aColor;

    out vec2 v_TexCoord;
    out vec4 v_Color;

    uniform mat4 u_Transform;

    void main() {
        v_TexCoord  = aTexCoord;
        v_Color     = aColor;
        gl_Position = u_Transform * vec4(aPos, 0.0, 1.0);
    }
)";

constexpr const char* fragment_shader_source = R"(
    #version 450 core

    in vec2 v_TexCoord;
    in vec4 v_Color;

    out vec4 FragColor;

    uniform vec4      u_Color;
    uniform sampler2D u_Texture;
    uniform bool      u_UseTexture;

void main() {
    if (u_UseTexture) {
        vec4 tex_color = texture(u_Texture, v_TexCoord);
        // テクスチャカラー * 頂点カラー * uniformカラー
        FragColor = tex_color * v_Color * u_Color;
    } else {
        FragColor = v_Color * u_Color;
    }
}
)";

using namespace std::string_view_literals;

class shader final {
  public:
    static constexpr auto u_Transform  = "u_Transform"sv;
    static constexpr auto u_Color      = "u_Color"sv;
    static constexpr auto u_UseTexture = "u_UseTexture"sv;
    static constexpr auto u_Texture    = "u_Texture"sv;

    [[nodiscard]] static auto try_make() noexcept -> std::expected<shader, error> {
        const auto vertex_shader = shader::compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
        if (not vertex_shader) return make_error(logic_error, "Failed to compile vertex shader.");

        const auto fragment_shader =
            shader::compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
        if (not fragment_shader)
            return make_error(logic_error, "Failed to compile fragment shader.");

        const auto program = glCreateProgram();
        glAttachShader(program, *vertex_shader);
        glAttachShader(program, *fragment_shader);
        glLinkProgram(program);
        if (GL_LINK_STATUS == GL_FALSE) return make_error(logic_error, "Failed to link.");
        glDeleteShader(*vertex_shader);
        glDeleteShader(*fragment_shader);

        return shader{program};
    }

    shader(const shader&) noexcept                    = delete;
    auto operator=(const shader&) noexcept -> shader& = delete;

    shader(shader&& other) noexcept : program_id_{std::exchange(other.program_id_, 0)} {}
    auto operator=(shader&& other) noexcept -> shader& {
        if (this == &other) return *this;

        destroy();

        program_id_ = std::exchange(other.program_id_, 0);
        return *this;
    }

    ~shader() noexcept { destroy(); }

    void use() noexcept {  // NOLINT
        if (program_id_ != 0) glUseProgram(program_id_);
    }

    // 以下はGPUに描画をセットする関数。直後にMesh::draw()が呼び出されるのを期待する
    void set_mat4(const std::string_view name, const glm::mat4& matrix) noexcept {  // NOLINT
        const auto location =
            glGetUniformLocation(program_id_, static_cast<const char*>(name.data()));
        glProgramUniformMatrix4fv(program_id_, location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void set_vec4(const std::string_view name, const glm::vec4& value) noexcept {  // NOLINT
        const auto location =
            glGetUniformLocation(program_id_, static_cast<const char*>(name.data()));
        glProgramUniform4fv(program_id_, location, 1, glm::value_ptr(value));
    }

    void set_int(const std::string_view name, const i32 value) noexcept {  // NOLINT
        const auto location =
            glGetUniformLocation(program_id_, static_cast<const char*>(name.data()));
        glProgramUniform1i(program_id_, location, value);
    }

    void set_bool(const std::string_view name, const bool value) noexcept {  // NOLINT
        const auto location =
            glGetUniformLocation(program_id_, static_cast<const char*>(name.data()));
        glProgramUniform1i(program_id_, location, static_cast<int>(value));
    }

  private:
    explicit shader(GLuint program_id) noexcept : program_id_{program_id} {}

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
        if (program_id_ == 0) return;
        glDeleteProgram(program_id_);
        program_id_ = 0;
    }

    GLuint program_id_{};
};

[[nodiscard]] inline auto try_make_shader() noexcept -> std::expected<shader, error> {
    return shader::try_make();
}
}  // namespace rin