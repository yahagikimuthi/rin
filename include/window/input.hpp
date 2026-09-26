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
#include "window/mouse.hpp"

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

    [[nodiscard]] auto is_down(const key key_button) const noexcept -> bool {
        const auto idx = key_to_size_t(key_button);
        return current_[idx];
    }

    [[nodiscard]] auto is_pressed(const key key_button) const noexcept -> bool {
        const auto idx = key_to_size_t(key_button);
        return current_[idx] and not previous_[idx];
    }

    [[nodiscard]] auto is_released(const key key_button) const noexcept -> bool {
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

class mouse_input final {
  public:
    [[nodiscard]] static auto make() noexcept -> mouse_input { return mouse_input{}; }

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

    [[nodiscard]] auto position(const extent window_size) const noexcept -> vec2 {
        if (window_size.width <= 0.f or window_size.height <= 0.f) return {.x = 0.f, .y = 0.f};

        const auto norm_x = position_.x / window_size.width;
        const auto norm_y = 1.f - (position_.y / window_size.height);

        return {.x = norm_x * virtual_window_size.width, .y = norm_y * virtual_window_size.height};
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
    explicit mouse_input() noexcept = default;

    [[nodiscard]] static auto to_size_t(const mouse button) noexcept -> std::size_t {
        return static_cast<std::size_t>(button);
    }

    std::array<bool, GLFW_MOUSE_BUTTON_LAST> curent_{};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST> previous_{};
    vec2                                     position_;
};

[[nodiscard]] inline auto make_mouse_input() noexcept -> mouse_input { return mouse_input::make(); }

class input final {
  public:
    [[nodiscard]] static auto make() noexcept -> input { return input{}; }

    void update(GLFWwindow* win) noexcept {
        if (win == nullptr) return;

        key_.update(win);
        mouse_.update(win);

        auto x = f64{};
        auto y = f64{};

        glfwGetCursorPos(win, &x, &y);
        mouse_position_ = {.x = static_cast<f32>(x), .y = static_cast<f32>(y)};
    }

    [[nodiscard]] auto is_key_down(const key key_button) const noexcept -> bool {
        return key_.is_down(key_button);
    }

    [[nodiscard]] auto is_key_pressed(const key key_button) const noexcept -> bool {
        return key_.is_pressed(key_button);
    }

    [[nodiscard]] auto is_key_released(const key key_button) const noexcept -> bool {
        return key_.is_released(key_button);
    }

    [[nodiscard]] auto is_mouse_down(const mouse button) const noexcept -> bool {
        return mouse_.is_down(button);
    }

    [[nodiscard]] auto is_mouse_pressed(const mouse button) const noexcept -> bool {
        return mouse_.is_pressed(button);
    }

    [[nodiscard]] auto is_released(const mouse button) const noexcept -> bool {
        return mouse_.is_released(button);
    }

    [[nodiscard]] auto mouse_position(const extent window_size) const noexcept -> vec2 {
        return mouse_.position(window_size);
    }

  private:
    explicit input() noexcept = default;
    [[nodiscard]] static auto key_to_size_t(const key key_button) noexcept -> std::size_t {
        return static_cast<std::size_t>(std::to_underlying(key_button));
    }

    key_input   key_{make_key_input()};
    mouse_input mouse_{make_mouse_input()};
    vec2        mouse_position_{};
};

[[nodiscard]] inline auto make_input() noexcept -> input { return input::make(); }
}  // namespace rin