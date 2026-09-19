#pragma once

#include <cassert>
#include <span>
#include <utility>

#include "glad/glad.h"

#include "type.hpp"

namespace rin {
class Mesh final {
  public:
    explicit Mesh(const std::span<const f32> vertices, const u32 vertex_count) noexcept {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);

        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertices.size() * sizeof(f32)),
            vertices.data(),
            GL_STATIC_DRAW
        );

        assert(vertices.size() % vertex_count == 0);
        const auto valuePerPoint = static_cast<i32>(vertices.size() / vertex_count);

        glVertexAttribPointer(
            0,
            valuePerPoint,
            GL_FLOAT,
            GL_FALSE,
            valuePerPoint * static_cast<i32>(sizeof(f32)),
            static_cast<void*>(0)
        );
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    Mesh(const Mesh&) noexcept                    = delete;
    auto operator=(const Mesh&) noexcept -> Mesh& = delete;
    Mesh(Mesh&& other) noexcept
        : vao_{std::exchange(other.vao_, 0)}, vbo_{std::exchange(other.vbo_, 0)} {}
    auto operator=(Mesh&& other) noexcept -> Mesh& {
        if (this == &other) return *this;

        if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
        if (vbo_ != 0) glDeleteBuffers(1, &vbo_);

        vao_ = std::exchange(other.vao_, 0);
        vbo_ = std::exchange(other.vbo_, 0);

        return *this;
    }
    ~Mesh() noexcept {
        if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
        if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    }

  private:
    GLuint vao_{};
    GLuint vbo_{};
};
}  // namespace rin