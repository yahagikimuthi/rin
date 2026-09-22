#pragma once

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>

#include "glad/glad.h"

#include "others/setting.hpp"
#include "others/type.hpp"

namespace rin {
struct PolygonIndexData final {
    GLuint  ebo{};
    GLsizei index_count{};
};

class EBOManager final {
  public:
    explicit EBOManager() noexcept = default;
    [[nodiscard]] auto get_or_create(const u32 points) noexcept -> PolygonIndexData {
        const auto actual_points = std::max(3u, points);

        auto& slot = slots_[actual_points];
        if (slot.ebo == 0) slot = create_index_data(actual_points);
        return slot;
    }
    EBOManager(const EBOManager&) noexcept                    = delete;
    auto operator=(const EBOManager&) noexcept -> EBOManager& = delete;
    EBOManager(EBOManager&& other) noexcept : slots_{other.slots_} {
        other.slots_.fill(PolygonIndexData{});
    }
    auto operator=(EBOManager&& other) noexcept -> EBOManager& {
        if (this == &other) return *this;

        for (auto slot : slots_)
            if (slot.ebo != 0) glDeleteBuffers(1, &slot.ebo);

        slots_ = other.slots_;

        other.slots_.fill(PolygonIndexData{});

        return *this;
    }
    ~EBOManager() noexcept {
        for (auto slot : slots_)
            if (slot.ebo != 0) glDeleteBuffers(1, &slot.ebo);
    }

  private:
    [[nodiscard]] static auto create_index_data(const u32 points) noexcept -> PolygonIndexData {
        auto indices = std::vector<u32>{};
        indices.reserve(points * 3uz);

        // 中心点(0)と、外周の頂点(i+1, i+2)を結んで三角形を作る
        for (const auto i : std::views::indices(points)) {
            const u32 current_vert = i + 1;
            const u32 next_vert    = ((i + 1) % points) + 1;  // 最後の頂点は最初の外周頂点(1)に戻る

            indices.emplace_back(0);             // 中心点
            indices.emplace_back(current_vert);  // 現在の外周頂点
            indices.emplace_back(next_vert);     // 次の外周頂点
        }

        auto ebo = GLuint{};
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() * sizeof(u32)),
            indices.data(),
            GL_STATIC_DRAW
        );
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        return PolygonIndexData{.ebo = ebo, .index_count = static_cast<GLsizei>(indices.size())};
    }

    std::array<PolygonIndexData, setting::default_circle_segments + 1> slots_{};
};
}  // namespace rin