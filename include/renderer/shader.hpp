#pragma once

#include <expected>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <string_view>
#include <utility>

#include "glad/glad.h"

#include "error.hpp"
#include "type.hpp"

namespace rin {
constexpr const char* vertex_shader_source = R"(
    #version 450 core
    
    layout (location = 0) in vec2 aPos; // 2Dゲーム用のため vec2 に変更

    uniform mat4 u_Transform;

    void main() {
        gl_Position = u_Transform * vec4(aPos, 0.0, 1.0);
    }
)";

constexpr const char* fragment_shader_source = R"(
    #version 450 core

    out vec4 FragColor;

    uniform vec4 u_Color;

    void main() {
        FragColor = u_Color;
    }
)";

using namespace std::string_view_literals;

class Shader final {
  public:
    static constexpr auto u_Transform = "u_Transform"sv;
    static constexpr auto u_Color     = "u_Color"sv;

    [[nodiscard]] static auto create() -> std::expected<Shader, Error> {
        const auto vertex = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
        if (not vertex) return Error::create(Error::logic, "Failed to Vertex Compile");

        const auto fragment = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
        if (not fragment) return Error::create(Error::logic, "Failed to Fragment Compile");

        const auto program = glCreateProgram();
        glAttachShader(program, *vertex);
        glAttachShader(program, *fragment);
        glLinkProgram(program);
        if (GL_LINK_STATUS == GL_FALSE) return Error::create(Error::logic, "Failed to Link");
        glDeleteShader(*vertex);
        glDeleteShader(*fragment);

        return Shader{program};
    }

    Shader(const Shader&) noexcept                    = delete;
    auto operator=(const Shader&) noexcept -> Shader& = delete;

    Shader(Shader&& other) noexcept : program_id_{std::exchange(other.program_id_, 0)} {}
    auto operator=(Shader&& other) noexcept -> Shader& {
        if (this == &other) return *this;

        glDeleteProgram(program_id_);
        program_id_ = std::exchange(other.program_id_, 0);
        return *this;
    }

    ~Shader() noexcept { glDeleteProgram(program_id_); }

    void use() const noexcept {
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

  private:
    explicit Shader(GLuint program_id) noexcept : program_id_{program_id} {}

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

    GLuint program_id_{};
};
}  // namespace rin