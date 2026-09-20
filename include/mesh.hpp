#pragma once

#include <cassert>
#include <expected>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include "glad/glad.h"

#include "type.hpp"
#include "util.hpp"

namespace rin {
class Mesh final {
  public:
    [[nodiscard]] static auto create(
        const std::span<const Vector2f> points,
        const std::span<const RGB>      colors,
        const std::span<const u32>      indices
    ) noexcept -> std::expected<Mesh, Error> {
        if (points.size() != colors.size())
            return Error::create(Error::logic, "Points and colors should same size");

        const auto               vertices_size = (3 * points.size()) + (3 * colors.size());
        static thread_local auto vertices      = std::vector<f32>{};
        vertices.clear();
        vertices.reserve(vertices_size);

        for (const auto i : std::views::indices(points.size())) {
            vertices.emplace_back(points[i].x);
            vertices.emplace_back(points[i].y);
            vertices.emplace_back(0.f);
            vertices.emplace_back(colors[i].r);
            vertices.emplace_back(colors[i].g);
            vertices.emplace_back(colors[i].b);
        }

        return Mesh{vertices, indices, 6};
    }

    [[nodiscard]] static auto create(
        const std::span<const Vector2f> points,
        const std::span<const RGB>      colors,
        const std::span<const UV>       uvs,
        const std::span<const u32>      indices
    ) noexcept -> std::expected<Mesh, Error> {
        if (points.size() != colors.size() or points.size() != uvs.size())
            return Error::create(Error::logic, "Points and Colors and UVs should same size");

        const auto vertices_size = (3 * points.size()) + (3 * colors.size()) + (2 * uvs.size());
        static thread_local auto vertices = std::vector<f32>{};
        vertices.clear();
        vertices.reserve(vertices_size);

        for (const auto i : std::views::indices(points.size())) {
            vertices.emplace_back(points[i].x);
            vertices.emplace_back(points[i].y);
            vertices.emplace_back(0.f);
            vertices.emplace_back(colors[i].r);
            vertices.emplace_back(colors[i].g);
            vertices.emplace_back(colors[i].b);
            vertices.emplace_back(uvs[i].u);
            vertices.emplace_back(uvs[i].v);
        }
        return Mesh{vertices, indices, 8};
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

    void draw() const noexcept {
        if (vao_ == 0) return;
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count_), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

  private:
    explicit Mesh(
        const std::span<const f32> vertices,
        const std::span<const u32> indices,
        const u32                  components_per_vertex
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

        // glVertexAttribPointerについて
        // 第一引数: location(座標や色などの属性を指示、Shaderのソースコードで設定)
        // 第二引数: 属性が数字何個で構成されるのかを指示, 色ならRGBで3個
        // 第三引数: 数字の型(バイト数を得るのに必要)
        // 第四引数: 正規化するか
        // 第五引数: 全部で何バイト存在するか
        // 第六引数: 配列の何バイト目から読み込むか(RGBなら最初は座標のあとだから座標3個+f32バイト)

        const auto vertex_size = static_cast<i32>(components_per_vertex * sizeof(f32));
        // 位置の設定
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            vertex_size,
            reinterpret_cast<void*>(0)  // NOLINT
        );
        glEnableVertexAttribArray(0);

        // 色の設定
        if (components_per_vertex > 3) {
            glVertexAttribPointer(
                1,
                3,
                GL_FLOAT,
                GL_FALSE,
                vertex_size,
                reinterpret_cast<void*>(3 * sizeof(f32))  // NOLINT
            );
            glEnableVertexAttribArray(1);
        }

        // UVの設定
        if (components_per_vertex > 5) {
            glVertexAttribPointer(
                2,
                2,
                GL_FLOAT,
                GL_FALSE,
                vertex_size,
                reinterpret_cast<void*>(6 * sizeof(f32))  // NOLINT
            );
            glEnableVertexAttribArray(2);
        }

        // GL_ELEMENT_ARRAY_BUFFER は VAO を解く前にアンバインドしてはいけない
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

    GLuint vao_{0};
    GLuint vbo_{0};
    GLuint ebo_{0};
    u32    index_count_{0};
};
}  // namespace rin