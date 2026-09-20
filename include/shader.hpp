#pragma once

#include <expected>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>

#include "glad/glad.h"

#include "type.hpp"

namespace rin {
// バーテックスシェーダー（頂点の位置をそのまま通過）
constexpr const char* vertex_shader_source = R"(
    #version 450 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 ourColor;

    void main() {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor;
    }
)";

// フラグメントシェーダー（ピクセルをオレンジ色に）
constexpr const char* fragment_shader_source = R"(
    #version 450 core
    in vec3 ourColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(ourColor, 1.0f);
    }
)";

class Shader final {
  public:
    enum class ErrorCode : u8 {
        failed_to_vertex_compile,
        failed_to_fragment_compile,
        failed_to_vertex_file_read,
        failed_to_fragment_file_read,
        failed_to_link
    };

    [[nodiscard]] static auto create() -> std::expected<Shader, ErrorCode> {
        const auto vertex = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
        if (not vertex) {
            std::cerr << "Failed to vertex compile" << '\n';
            return std::unexpected{ErrorCode::failed_to_vertex_compile};
        }

        const auto fragment = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
        if (not fragment) {
            std::cerr << "Failed to fragment compile" << '\n';
            return std::unexpected{ErrorCode::failed_to_fragment_compile};
        }

        const auto program = glCreateProgram();
        glAttachShader(program, *vertex);
        glAttachShader(program, *fragment);
        glLinkProgram(program);
        if (GL_LINK_STATUS == GL_FALSE) {
            std::cerr << "Failed to link" << '\n';
            return std::unexpected{ErrorCode::failed_to_link};
        }
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