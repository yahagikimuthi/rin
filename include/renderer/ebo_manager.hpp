#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <ranges>
#include <vector>

#include "glad/glad.h"

#include "setting.hpp"
#include "type.hpp"

namespace rin {
struct CircleIndexData final {
    GLuint  ebo{};
    GLsizei index_count{};
};

class EBOManager final {
  public:
    explicit EBOManager() noexcept = default;
    [[nodiscard]] auto get_or_create(const u32 sides) noexcept -> CircleIndexData {
        const auto actual_sides = std::max(3u, sides);

        auto& slot = slots_[actual_sides];
        if (slot.ebo == 0) slot = create_index_data(sides);
        return slot;
    }
    EBOManager(const EBOManager&) noexcept                    = delete;
    auto operator=(const EBOManager&) noexcept -> EBOManager& = delete;
    EBOManager(EBOManager&& other) noexcept : slots_{other.slots_} {
        other.slots_.fill(CircleIndexData{});
    }
    auto operator=(EBOManager&& other) noexcept -> EBOManager& {
        if (this == &other) return *this;

        for (auto slot : slots_)
            if (slot.ebo != 0) glDeleteBuffers(1, &slot.ebo);

        slots_ = other.slots_;

        other.slots_.fill(CircleIndexData{});

        return *this;
    }
    ~EBOManager() noexcept {
        for (auto slot : slots_)
            if (slot.ebo != 0) glDeleteBuffers(1, &slot.ebo);
    }

  private:
    static auto create_index_data(const u32 sides) noexcept -> CircleIndexData {
        auto indices = std::vector<u32>{};
        indices.reserve(sides * 3uz);

        for (const auto i : std::views::indices(sides)) {
            const auto current_vert =
                static_cast<u32>(1 + std::round((setting::default_circle_segments * i) / sides)) %
                setting::default_circle_segments;
            const auto next_vert =
                static_cast<u32>(
                    1 + std::round(
                            static_cast<f32>(setting::default_circle_segments * (i + 1)) /
                            static_cast<f32>(sides)
                        )
                ) %
                setting::default_circle_segments;
            indices.emplace_back(0);
            indices.emplace_back(current_vert);
            indices.emplace_back(next_vert);
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

        return CircleIndexData{.ebo = ebo, .index_count = static_cast<GLsizei>(indices.size())};
    }

    std::array<CircleIndexData, setting::default_circle_segments + 1> slots_{};
};
}  // namespace rin