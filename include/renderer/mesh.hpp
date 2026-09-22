#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <numbers>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include "glad/glad.h"

#include "type.hpp"

namespace rin {
class Mesh final {
  public:
    explicit Mesh(const u32 segments) noexcept
        : Mesh(create_vertices(segments), create_indices(segments)) {}
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

    void draw() const noexcept { draw(ebo_, index_count_); }
    void draw(GLuint custom_ebo, GLsizei count) const noexcept {
        if (vao_ == 0 or custom_ebo == 0) return;

        glBindVertexArray(vao_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, custom_ebo);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBindVertexArray(0);
    }

  private:
    [[nodiscard]] static auto create_vertices(const u32 segments) noexcept -> std::vector<f32> {
        const auto segs = std::max(3u, segments);

        auto vertices = std::vector<f32>{};
        vertices.reserve((segs + 1uz) * 2uz);

        // 中心点
        vertices.emplace_back(0.f);
        vertices.emplace_back(0.f);

        // 円周点
        constexpr auto radius = 0.5f;
        for (const auto i : std::views::iota(0u, segs)) {
            const auto angle =
                (2.f * std::numbers::pi_v<f32> * static_cast<f32>(i)) / static_cast<f32>(segs);
            vertices.emplace_back(radius * std::cos(angle));
            vertices.emplace_back(radius * std::sin(angle));
        }
        return vertices;
    }

    [[nodiscard]] static auto create_indices(const u32 segments) noexcept -> std::vector<u32> {
        const auto segs = std::max(3u, segments);

        auto indices = std::vector<u32>{};
        indices.reserve(segs * 3uz);

        for (const auto i : std::views::iota(0u, segs)) {
            const auto current = static_cast<u32>(i + 1);
            const auto next    = static_cast<u32>(1 + ((i + 1) % segs));

            indices.emplace_back(0u);
            indices.emplace_back(current);
            indices.emplace_back(next);
        }

        return indices;
    }

    explicit Mesh(const std::span<const f32> vertices, const std::span<const u32> indices) noexcept
        : index_count_{static_cast<i32>(indices.size())} {
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

        // 頂点アトリビュート設定 (2D: x, y)
        constexpr i32 components_per_vertex = 2;
        const auto    stride                = static_cast<i32>(components_per_vertex * sizeof(f32));

        glVertexAttribPointer(
            0,
            components_per_vertex,
            GL_FLOAT,
            GL_FALSE,
            stride,
            reinterpret_cast<void*>(0)  // NOLINT
        );
        glEnableVertexAttribArray(0);

        // VAO のバインド解除 (EBOは解く前にVAOをアンバインドする)
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void destroy() noexcept {
        if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
        if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
        if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
        index_count_ = 0;
    }

    GLuint vao_{};
    GLuint vbo_{};
    GLuint ebo_{};
    i32    index_count_{};
};
}  // namespace rin