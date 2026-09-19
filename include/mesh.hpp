#pragma once

#include <cassert>
#include <span>
#include <utility>

#include "glad/glad.h"

#include "type.hpp"

namespace rin {
class Mesh final {
  public:
    explicit Mesh(
        const std::span<const f32> vertices,
        const std::span<const u32> indices,
        const u32                  vertex_count
    ) noexcept
        : index_count_{static_cast<u32>(indices.size())} {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        // VBO設定
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertices.size() * sizeof(f32)),
            vertices.data(),
            GL_STATIC_DRAW
        );

        // EBO設定
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() * sizeof(u32)),
            indices.data(),
            GL_STATIC_DRAW
        );

        assert(vertex_count > 0 and vertices.size() % vertex_count == 0);
        const auto value_per_point = static_cast<i32>(vertices.size() / vertex_count);

        glVertexAttribPointer(
            0,
            value_per_point,
            GL_FLOAT,
            GL_FALSE,
            value_per_point * static_cast<i32>(sizeof(f32)),
            static_cast<void*>(0)
        );
        glEnableVertexAttribArray(0);

        // GL_ELEMENT_ARRAY_BUFFER は VAO を解く前にアンバインドしてはいけない！
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    Mesh(const Mesh&) noexcept                    = delete;
    auto operator=(const Mesh&) noexcept -> Mesh& = delete;

    Mesh(Mesh&& other) noexcept
        : vao_{std::exchange(other.vao_, 0)},
          vbo_{std::exchange(other.vbo_, 0)},
          ebo_{std::exchange(other.ebo_, 0)},
          index_count_{std::exchange(other.index_count_, 0)} {}

    auto operator=(Mesh&& other) noexcept -> Mesh& {
        if (this == &other) return *this;

        destroy();

        vao_         = std::exchange(other.vao_, 0);
        vbo_         = std::exchange(other.vbo_, 0);
        ebo_         = std::exchange(other.ebo_, 0);
        index_count_ = std::exchange(other.index_count_, 0);

        return *this;
    }

    ~Mesh() noexcept { destroy(); }

    // 描画メソッド（EBOを使用した glDrawElements）
    void draw() const noexcept {
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count_), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

  private:
    void destroy() noexcept {
        if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
        if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
        if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
    }

    GLuint vao_{0};
    GLuint vbo_{0};
    GLuint ebo_{0};
    u32    index_count_{0};
};
}  // namespace rin