#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <glm/ext/vector_float2.hpp>
#include <ranges>

#include "detail/graphics.hpp"
#include "extent.hpp"
#include "key.hpp"
#include "mouse.hpp"
#include "types.hpp"
#include "vec2.hpp"

namespace rin::detail {
struct view_point final {
    i32 x;
    i32 y;
    i32 w;
    i32 h;
};

class key_input final {
  public:
    explicit key_input() noexcept = default;

    void update(GLFWwindow* win) noexcept {
        previous_ = current_;

        for (const auto i : std::views::indices(current_.size())) {
            current_[i] = glfwGetKey(win, static_cast<i32>(i)) == GLFW_PRESS;
        }
    }

    [[nodiscard]] auto is_down(const key button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return current_[idx];
    }

    [[nodiscard]] auto is_pressed(const key button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return current_[idx] and not previous_[idx];
    }

    [[nodiscard]] auto is_released(const key button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return not current_[idx] and previous_[idx];
    }

  private:
    [[nodiscard]] static auto to_size_t(const key button) noexcept -> std::size_t {
        return static_cast<std::size_t>(button);
    }

    std::array<bool, GLFW_KEY_LAST> current_{};
    std::array<bool, GLFW_KEY_LAST> previous_{};
};

class mouse_input final {
  public:
    explicit mouse_input() noexcept = default;

    void update(GLFWwindow* win) noexcept {
        previous_ = curent_;

        for (const auto i : std::views::indices(curent_.size())) {
            curent_[i] = glfwGetMouseButton(win, static_cast<i32>(i)) == GLFW_PRESS;
        }

        auto x = f64{};
        auto y = f64{};

        glfwGetCursorPos(win, &x, &y);
        position_ = {.x = static_cast<f32>(x), .y = static_cast<f32>(y)};
    }

    [[nodiscard]] auto position(
        const view_point& vp, const extent& virtual_window_size
    ) const noexcept -> vec2 {
        if (vp.w <= 0 or vp.h <= 0) {
            return vec2{.x = 0.f, .y = 0.f};
        }

        const auto relative_x = position_.x - static_cast<f32>(vp.x);
        const auto relative_y = position_.y - static_cast<f32>(vp.y);

        const auto norm_x = relative_x / static_cast<f32>(vp.w);

        const auto norm_y = 1.f - (relative_y / static_cast<f32>(vp.h));

        const auto virt_x = norm_x * virtual_window_size.width;
        const auto virt_y = norm_y * virtual_window_size.height;

        return vec2{.x = virt_x, .y = virt_y};
    }

    [[nodiscard]] auto is_down(const mouse button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return curent_[idx];
    }

    [[nodiscard]] auto is_pressed(const mouse button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return curent_[idx] and not previous_[idx];
    }

    [[nodiscard]] auto is_released(const mouse button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return not curent_[idx] and previous_[idx];
    }

  private:
    [[nodiscard]] static auto to_size_t(const mouse button) noexcept -> std::size_t {
        return static_cast<std::size_t>(button);
    }

    std::array<bool, GLFW_MOUSE_BUTTON_LAST> curent_{};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST> previous_{};
    vec2                                     position_;
};

class input final {
  public:
    explicit input() noexcept = default;

    void update(GLFWwindow* win) noexcept {
        if (win == nullptr) return;

        key_.update(win);
        mouse_.update(win);
    }

    [[nodiscard]] auto is_key_down(const key button) const noexcept -> bool {
        return key_.is_down(button);
    }

    [[nodiscard]] auto is_key_pressed(const key button) const noexcept -> bool {
        return key_.is_pressed(button);
    }

    [[nodiscard]] auto is_key_released(const key button) const noexcept -> bool {
        return key_.is_released(button);
    }

    [[nodiscard]] auto is_mouse_down(const mouse button) const noexcept -> bool {
        return mouse_.is_down(button);
    }

    [[nodiscard]] auto is_mouse_pressed(const mouse button) const noexcept -> bool {
        return mouse_.is_pressed(button);
    }

    [[nodiscard]] auto is_mouse_released(const mouse button) const noexcept -> bool {
        return mouse_.is_released(button);
    }

    [[nodiscard]] auto mouse_position(
        const view_point& vp, const extent& virtual_window_size
    ) const noexcept -> vec2 {
        return mouse_.position(vp, virtual_window_size);
    }

  private:
    key_input   key_;
    mouse_input mouse_;
};

}  // namespace rin::detail