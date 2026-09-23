#pragma once

#include <cassert>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <span>
#include <utility>

#include "glad/glad.h"

#include "others/type.hpp"

namespace rin {
class mesh final {
  public:
    explicit mesh(const u32 max_vertices) noexcept : max_vertices_{max_vertices} {
        glCreateVertexArrays(1, &vao_);
        glCreateBuffers(1, &vbo_);

        // バッファ構築
        const auto total_bytes = static_cast<GLsizeiptr>(max_vertices * sizeof(glm::vec2));
        glNamedBufferData(vbo_, total_bytes, nullptr, GL_DYNAMIC_DRAW);

        constexpr auto attrib_index  = GLuint{};
        constexpr auto binding_index = GLuint{};
        constexpr auto components    = 2;

        glEnableVertexArrayAttrib(vao_, attrib_index);
        glVertexArrayAttribFormat(vao_, attrib_index, components, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao_, attrib_index, binding_index);

        glVertexArrayVertexBuffer(
            vao_, binding_index, vbo_, 0, static_cast<GLsizei>(sizeof(glm::vec2))
        );
    }
    mesh(const mesh&)                             = delete;
    auto operator=(const mesh&) noexcept -> mesh& = delete;

    mesh(mesh&& other) noexcept
        : vao_{std::exchange(other.vao_, 0)},
          vbo_{std::exchange(other.vbo_, 0)},
          max_vertices_{std::exchange(other.max_vertices_, 0)} {}
    auto operator=(mesh&& other) noexcept -> mesh& {
        if (this == &other) return *this;

        destroy();

        vao_          = std::exchange(other.vao_, 0);
        vbo_          = std::exchange(other.vbo_, 0);
        max_vertices_ = std::exchange(other.max_vertices_, 0);

        return *this;
    }

    ~mesh() noexcept { destroy(); }

    void update_vertices(const std::span<const glm::vec2> vertices) noexcept {  // NOLINT
        if (vbo_ == 0 or vertices.empty()) return;

        assert(
            vertices.size() <= max_vertices_ and
            "Requires vertices is greater than this Mesh vertices buffer. Please construct by "
            "larger vertices buffer."
        );

        const auto upload_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec2));
        glNamedBufferSubData(vbo_, 0, upload_size, vertices.data());
    }

    void draw(const GLuint ebo, const GLsizei index_count) const noexcept {
        if (vao_ == 0 or ebo == 0) return;

        glVertexArrayElementBuffer(vao_, ebo);
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
    }

  private:
    void destroy() noexcept {
        if (vbo_ != 0) {
            glDeleteBuffers(1, &vbo_);
            vbo_ = 0;
        }
        if (vao_ != 0) {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
        max_vertices_ = 0;
    }

    GLuint vao_{};
    GLuint vbo_{};
    u32    max_vertices_{};
};
}  // namespace rin