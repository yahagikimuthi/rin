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

class ebo_manager {
  public:
    [[nodiscard]] static auto make() noexcept -> ebo_manager { return ebo_manager{}; }

    [[nodiscard]] auto get_or_create(const u32 vertex_count) noexcept -> polygon_index_data {
        const auto actual_vertex = std::max(vertex_count, 3u);

        if (actual_vertex <= default_circle_segments) {
            auto& slot = default_slots_[actual_vertex];
            if (slot.ebo == 0) slot = create_index_data(actual_vertex);
            return slot;
        }

        unexpected_slots_.resize(actual_vertex + 1);
        auto& slot = unexpected_slots_[actual_vertex];
        if (slot.ebo == 0) slot = create_index_data(actual_vertex);
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
    explicit ebo_manager() noexcept = default;

    [[nodiscard]] static auto create_index_data(const u32 vertex_count) noexcept
        -> polygon_index_data {
        const auto triangle_cnt = vertex_count - 2;
        const auto index_cnt    = triangle_cnt * 3;

        static thread_local auto indices = std::vector<u32>{};
        indices.clear();
        indices.reserve(index_cnt);

        for (const auto i : std::views::iota(1uz, triangle_cnt + 1)) {
            indices.emplace_back(0);
            indices.emplace_back(i);
            indices.emplace_back(i + 1);
        }

        auto ebo = GLuint{};
        glCreateBuffers(1, &ebo);

        const auto buffer_size = static_cast<GLsizeiptr>(indices.size() * sizeof(u32));
        glNamedBufferData(ebo, buffer_size, indices.data(), GL_STATIC_DRAW);

        return {.ebo = ebo, .index_count = static_cast<GLsizei>(indices.size())};
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

[[nodiscard]] inline auto make_ebo_manager() noexcept -> ebo_manager { return ebo_manager::make(); }
}  // namespace rin