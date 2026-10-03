#pragma once

#include <array>
#include <cstddef>
#include <ranges>

#include "rin/color.hpp"
#include "rin/vertex_vector.hpp"
#include "rin/window.hpp"

#include "examples/breakout/setting.hpp"

namespace gm {
struct Block final {
    rin::vec2 position;
    bool      is_valid{true};
};

class BlockVec final {
  public:
    explicit BlockVec() noexcept = default;

    [[nodiscard]] auto operator[](const std::size_t x, const std::size_t y) const noexcept
        -> const Block& {
        return positions_[x + (block_row * y)];
    }
    [[nodiscard]] auto operator[](const std::size_t x, const std::size_t y) noexcept -> Block& {
        return positions_[x + (block_row * y)];
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return positions_.size(); }

  private:
    std::array<Block, static_cast<std::size_t>(block_row* block_col)> positions_{};
};

class Blocks final {
  public:
    explicit Blocks() noexcept {
        vertices_.reserve(4);

        constexpr auto half_w = block_size.width / 2;
        constexpr auto half_h = block_size.height / 2;

        constexpr auto p1 = rin::vec2{.x = -half_w, .y = half_h};
        constexpr auto p2 = rin::vec2{.x = -half_w, .y = -half_h};
        constexpr auto p3 = rin::vec2{.x = half_w, .y = -half_h};
        constexpr auto p4 = rin::vec2{.x = half_w, .y = half_h};

        vertices_.emplace_back(p1, rin::white);
        vertices_.emplace_back(p2, rin::white);
        vertices_.emplace_back(p3, rin::white);

        vertices_.emplace_back(p3, rin::white);
        vertices_.emplace_back(p4, rin::white);
        vertices_.emplace_back(p1, rin::white);

        for (const auto row_idx : std::views::indices(static_cast<std::size_t>(block_row))) {
            const auto x = block_side_blank +
                           (static_cast<f32>(row_idx) * (block_size.width + block_margin)) + half_w;
            for (const auto col_idx : std::views::indices(static_cast<std::size_t>(block_col))) {
                const auto y =
                    window_size.height -
                    (block_side_blank +
                     (static_cast<f32>(col_idx) * (block_size.height + block_margin)) + half_h);
                vec[row_idx, col_idx].position = rin::vec2{.x = x, .y = y};
            }
        }
    }

    void draw(rin::window& window) noexcept {
        for (const auto row_idx : std::views::indices(static_cast<std::size_t>(block_row))) {
            for (const auto col_idx : std::views::indices(static_cast<std::size_t>(block_col))) {
                if (not vec[row_idx, col_idx].is_valid) continue;

                const auto position = vec[row_idx, col_idx].position;
                vertices_.position(position);
                vertices_.color(block_col_colors[col_idx]);
                window.draw(vertices_);
            }
        }
    }

    [[nodiscard]] auto is_any_blocks_valid() const noexcept -> bool {
        for (const auto row_idx : std::views::indices(static_cast<std::size_t>(block_row))) {
            for (const auto col_idx : std::views::indices(static_cast<std::size_t>(block_col))) {
                const auto& block = vec[row_idx, col_idx];
                if (block.is_valid) return true;
            }
        }
        return false;
    }

    [[nodiscard]] auto calc_invalid_blocks() const noexcept -> u32 {
        auto cnt = u32{};
        for (const auto row_idx : std::views::indices(static_cast<std::size_t>(block_row))) {
            for (const auto col_idx : std::views::indices(static_cast<std::size_t>(block_col))) {
                const auto& block = vec[row_idx, col_idx];
                if (not block.is_valid) ++cnt;
            }
        }
        return cnt;
    }

    void reset() noexcept {
        for (const auto row_idx : std::views::indices(static_cast<std::size_t>(block_row))) {
            for (const auto col_idx : std::views::indices(static_cast<std::size_t>(block_col))) {
                auto& block    = vec[row_idx, col_idx];
                block.is_valid = true;
            }
        }
    }

    BlockVec vec;

  private:
    rin::vertex_vector vertices_{rin::primitive_triangles};
};
}  // namespace gm