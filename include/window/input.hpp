#pragma once

#include <array>
#include <cstddef>
#include <glm/ext/vector_float2.hpp>
#include <ranges>
#include <utility>

#include "GLFW/glfw3.h"
#include "others/type.hpp"
#include "others/util.hpp"
#include "window/key.hpp"

namespace rin {
class input final {
  public:
    explicit constexpr input() noexcept = default;

    void update(GLFWwindow* window) noexcept {
        if (window == nullptr) return;

        previous_keys_ = current_keys_;

        for (const auto i : std::views::indices(current_keys_.size())) {
            current_keys_[i] = (glfwGetKey(window, static_cast<int>(i)) == GLFW_PRESS);
        }

        auto x = f64{};
        auto y = f64{};

        glfwGetCursorPos(window, &x, &y);
        mouse_position_ = {.x = static_cast<f32>(x), .y = static_cast<f32>(y)};
    }

    [[nodiscard]] constexpr auto is_key_down(const key key_button) const noexcept -> bool {
        const auto idx = key_to_size_t(key_button);
        return current_keys_[idx];
    }
    [[nodiscard]] constexpr auto is_key_pressed(const key key_button) const noexcept -> bool {
        const auto idx = key_to_size_t(key_button);
        return current_keys_[idx] and not previous_keys_[idx];
    }
    [[nodiscard]] constexpr auto is_key_released(const key key_button) const noexcept -> bool {
        const auto idx = key_to_size_t(key_button);
        return not current_keys_[idx] and previous_keys_[idx];
    }
    [[nodiscard]] constexpr auto mouse_position() const noexcept -> vec2 { return mouse_position_; }

  private:
    [[nodiscard]] static constexpr auto key_to_size_t(const key key_button) noexcept
        -> std::size_t {
        return static_cast<std::size_t>(std::to_underlying(key_button));
    }

    std::array<bool, GLFW_KEY_LAST + 1> current_keys_{};
    std::array<bool, GLFW_KEY_LAST + 1> previous_keys_{};
    vec2                                mouse_position_{};
};
}  // namespace rin