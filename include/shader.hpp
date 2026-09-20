#pragma once

#include <expected>
#include <optional>
#include <string_view>
#include <utility>

#include "glad/glad.h"

#include "error.hpp"

namespace rin {
constexpr const char* vertex_shader_source = R"(
    #version 450 core
    
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    layout (location = 2) in vec2 aTexCoord;

    out vec3 ourColor;
    out vec2 TexCoord;

    void main() {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor;
        TexCoord = aTexCoord;
    }
)";

constexpr const char* fragment_shader_source = R"(
    #version 450 core
    in vec3 ourColor;
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform sampler2D ourTexture;

    void main() {
        FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
    }
)";

class Shader final {
  public:
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