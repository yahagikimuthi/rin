#pragma once

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>

#include "glad/glad.h"

#include "others/setting.hpp"
#include "others/type.hpp"

namespace rin {
struct polygon_index_data final {
    GLuint  ebo{};
    GLsizei index_count{};
};

class ebo_manager final {
  public:
    explicit ebo_manager() noexcept = default;
    [[nodiscard]] auto get_or_create(const u32 points) noexcept -> polygon_index_data {
        const auto actual_points = std::max(3u, points);
        if (actual_points <= default_circle_segments) {
            auto& slot = default_slots_[actual_points];
            if (slot.ebo == 0) slot = create_index_data(actual_points);
            return slot;
        }
        unexpected_slots_.resize(actual_points + 1);
        auto& slot = unexpected_slots_[actual_points];
        if (slot.ebo == 0) slot = create_index_data(actual_points);
        return slot;
    }

    ebo_manager(const ebo_manager&) noexcept                    = delete;
    auto operator=(const ebo_manager&) noexcept -> ebo_manager& = delete;
    ebo_manager(ebo_manager&& other) noexcept
        : default_slots_{other.default_slots_},
          unexpected_slots_{std::move(other.unexpected_slots_)} {
        other.default_slots_.fill(polygon_index_data{});
    }
    auto operator=(ebo_manager&& other) noexcept -> ebo_manager& {
        if (this == &other) return *this;

        destroy();

        default_slots_    = other.default_slots_;
        unexpected_slots_ = std::move(other.unexpected_slots_);

        other.default_slots_.fill(polygon_index_data{});

        return *this;
    }
    ~ebo_manager() noexcept { destroy(); }

  private:
    [[nodiscard]] static auto create_index_data(const u32 points) noexcept -> polygon_index_data {
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

        return polygon_index_data{.ebo = ebo, .index_count = static_cast<GLsizei>(indices.size())};
    }

    void destroy() noexcept {
        for (auto slot : default_slots_) {
            if (slot.ebo != 0) {
                glDeleteBuffers(1, &slot.ebo);
                slot.ebo = 0;
            }
        }

        for (auto slot : unexpected_slots_) {
            if (slot.ebo != 0) {
                glDeleteBuffers(1, &slot.ebo);
                slot.ebo = 0;
            }
        }
    }

    std::array<polygon_index_data, default_circle_segments + 1> default_slots_{};
    std::vector<polygon_index_data>                             unexpected_slots_;
};
}  // namespace rin