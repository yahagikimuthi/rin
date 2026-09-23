#pragma once

#include <cassert>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <span>
#include <utility>

#include "glad/glad.h"

#include "others/type.hpp"
#include "vertex.hpp"

namespace rin {
class mesh final {
  public:
    explicit mesh(const u32 max_vertices) noexcept : max_vertices_{max_vertices} {
        glCreateVertexArrays(1, &vao_);
        glCreateBuffers(1, &vbo_);

        // バッファ領域の確保 (vertex のサイズ × 最大頂点数)
        const auto total_bytes = static_cast<GLsizeiptr>(max_vertices * sizeof(vertex));
        glNamedBufferData(vbo_, total_bytes, nullptr, GL_DYNAMIC_DRAW);

        constexpr auto binding_index = GLuint{0};

        // location0 aPos (vec2)
        constexpr auto pos_attrib = GLuint{0};
        glEnableVertexArrayAttrib(vao_, pos_attrib);
        glVertexArrayAttribFormat(
            vao_, pos_attrib, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, position)
        );
        glVertexArrayAttribBinding(vao_, pos_attrib, binding_index);

        // location1 aTexCoord (uv)
        constexpr auto tex_attrib = GLuint{1};
        glEnableVertexArrayAttrib(vao_, tex_attrib);
        glVertexArrayAttribFormat(
            vao_, tex_attrib, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, tex_coord)
        );
        glVertexArrayAttribBinding(vao_, tex_attrib, binding_index);

        // Location2 aColor (color / vec4)
        constexpr auto col_attrib = GLuint{2};
        glEnableVertexArrayAttrib(vao_, col_attrib);
        glVertexArrayAttribFormat(vao_, col_attrib, 4, GL_FLOAT, GL_FALSE, offsetof(vertex, color));
        glVertexArrayAttribBinding(vao_, col_attrib, binding_index);

        // VAO のバインディングポイント 0 に VBO を接続 (ストライドは sizeof(vertex))
        glVertexArrayVertexBuffer(
            vao_,
            binding_index,
            vbo_,
            static_cast<GLintptr>(0),
            static_cast<GLsizei>(sizeof(vertex))
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

    void update_vertices(const std::span<const vertex> vertices) const noexcept {
        if (vbo_ == 0 or vertices.empty()) return;

        assert(
            vertices.size() <= max_vertices_ and
            "Requires vertices is greater than this Mesh vertices buffer. Please construct by "
            "larger vertices buffer."
        );

        const auto upload_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(vertex));
        glNamedBufferSubData(vbo_, 0, upload_size, vertices.data());
    }

    void draw(
        const GLuint ebo, const GLsizei index_count, const primitive_type type
    ) const noexcept {
        if (vao_ == 0 or ebo == 0) return;

        glVertexArrayElementBuffer(vao_, ebo);
        glBindVertexArray(vao_);
        glDrawElements(static_cast<GLenum>(type), index_count, GL_UNSIGNED_INT, nullptr);
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