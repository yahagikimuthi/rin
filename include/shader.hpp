#pragma once

#include <expected>
#include <optional>
#include <string_view>
#include <utility>

#include "glad/glad.h"

#include "type.hpp"
#include "util.hpp"

namespace rin {
class Shader final {
  public:
    enum class ErrorCode : u8 {
        vertex_compile_failed,
        fragment_compile_failed,
        vertex_file_read_failed,
        fragment_file_read_failed,
        link_failed
    };

    enum class ShaderType : u8 { vertex, fragment };

    [[nodiscard]] static auto createFromSource(
        std::string_view vert_source, std::string_view frag_source
    ) noexcept -> std::expected<Shader, ErrorCode> {
        const auto vertex = compile_shader(GL_VERTEX_SHADER, vert_source);
        if (not vertex) return std::unexpected{ErrorCode::vertex_compile_failed};

        const auto fragment = compile_shader(GL_FRAGMENT_SHADER, frag_source);
        if (not fragment) return std::unexpected{ErrorCode::fragment_compile_failed};

        const auto program = glCreateProgram();
        glAttachShader(program, *vertex);
        glAttachShader(program, *fragment);
        glLinkProgram(program);
        if (GL_LINK_STATUS == GL_FALSE) return std::unexpected{ErrorCode::link_failed};

        glDeleteShader(*vertex);
        glDeleteShader(*fragment);

        return Shader{program};
    }

    [[nodiscard]] static auto createFromFile(
        std::string_view vert_path, std::string_view frag_path
    ) noexcept -> std::expected<Shader, ErrorCode> {
        const auto vertex = read_file(vert_path);
        if (not vertex) return std::unexpected{ErrorCode::vertex_file_read_failed};

        const auto fragment = read_file(frag_path);
        if (not fragment) return std::unexpected{ErrorCode::fragment_file_read_failed};

        return createFromSource(*vertex, *fragment);
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