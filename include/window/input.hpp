#pragma once

#include <array>
#include <cstddef>
#include <glm/ext/vector_float2.hpp>
#include <ranges>
#include <utility>

#include "GLFW/glfw3.h"
#include "others/setting.hpp"
#include "others/type.hpp"
#include "others/util.hpp"
#include "window/key.hpp"

namespace rin {
class key_input final {
  public:
    [[nodiscard]] static auto make() noexcept -> key_input { return key_input{}; }

    void update(GLFWwindow* win) noexcept {
        previous_ = current_;

        for (const auto i : std::views::indices(current_.size())) {
            current_[i] = (glfwGetKey(win, static_cast<i32>(i))) == GLFW_PRESS;
        }
    }

    [[nodiscard]] auto is_key_down(const key key_button) const noexcept -> bool {
        const auto idx = key_to_size_t(key_button);
        return current_[idx];
    }

    [[nodiscard]] auto is_key_pressed(const key key_button) const noexcept -> bool {
        const auto idx = key_to_size_t(key_button);
        return current_[idx] and not previous_[idx];
    }

    [[nodiscard]] auto is_key_released(const key key_button) const noexcept -> bool {
        const auto idx = key_to_size_t(key_button);
        return not current_[idx] and previous_[idx];
    }

  private:
    explicit key_input() noexcept = default;

    [[nodiscard]] static auto key_to_size_t(const key key_button) noexcept -> std::size_t {
        return static_cast<std::size_t>(std::to_underlying(key_button));
    }

    std::array<bool, GLFW_KEY_LAST> current_{};
    std::array<bool, GLFW_KEY_LAST> previous_{};
};

[[nodiscard]] inline auto make_key_input() noexcept -> key_input { return key_input::make(); }

class input final {
  public:
    [[nodiscard]] static auto make() noexcept -> input { return input{}; }

    void update(GLFWwindow* win) noexcept {
        if (win == nullptr) return;

        key_input_.update(win);

        auto x = f64{};
        auto y = f64{};

        glfwGetCursorPos(win, &x, &y);
        mouse_position_ = {.x = static_cast<f32>(x), .y = static_cast<f32>(y)};
    }

    [[nodiscard]] auto is_key_down(const key key_button) const noexcept -> bool {
        return key_input_.is_key_down(key_button);
    }

    [[nodiscard]] auto is_key_pressed(const key key_button) const noexcept -> bool {
        return key_input_.is_key_pressed(key_button);
    }

    [[nodiscard]] auto is_key_released(const key key_button) const noexcept -> bool {
        return key_input_.is_key_released(key_button);
    }

    [[nodiscard]] auto mouse_position(const extent& window_size) const noexcept -> vec2 {
        if (window_size.width <= 0.f or window_size.height <= 0.f) return {.x = 0.f, .y = 0.f};

        const auto norm_x = mouse_position_.x / window_size.width;
        const auto norm_y = 1.f - (mouse_position_.y / window_size.height);

        return {.x = norm_x * virtual_window_size.width, .y = norm_y * virtual_window_size.height};
    }

  private:
    explicit input() noexcept = default;
    [[nodiscard]] static auto key_to_size_t(const key key_button) noexcept -> std::size_t {
        return static_cast<std::size_t>(std::to_underlying(key_button));
    }

    key_input key_input_{make_key_input()};
    vec2      mouse_position_{};
};

[[nodiscard]] inline auto make_input() noexcept -> input { return input::make(); }
}  // namespace rin