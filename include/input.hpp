#pragma once

#include <array>
#include <cstddef>
#include <glm/ext/vector_float2.hpp>
#include <ranges>
#include <utility>

#include "GLFW/glfw3.h"
#include "key.hpp"
#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
class input final {
  public:
    static void update(GLFWwindow* window) noexcept {
        if (window == nullptr) return;

        previous_keys_ = current_keys_;

        for (const auto i : std::views::indices(current_keys_.size())) {
            current_keys_[i] = (glfwGetKey(window, static_cast<int>(i)) == GLFW_PRESS);  // NOLINT
        }

        auto x = f64{};
        auto y = f64{};

        glfwGetCursorPos(window, &x, &y);
        mouse_position_ = {.x = static_cast<f32>(x), .y = static_cast<f32>(y)};
    }

    [[nodiscard]] static constexpr auto is_key_down(const key key) noexcept -> bool {
        const auto idx = key_to_size_t(key);
        return current_keys_[idx];
    }
    [[nodiscard]] static constexpr auto is_key_pressed(const key key) noexcept -> bool {
        const auto idx = key_to_size_t(key);
        return current_keys_[idx] and not previous_keys_[idx];
    }
    [[nodiscard]] static constexpr auto is_key_released(const key key) noexcept -> bool {
        const auto idx = key_to_size_t(key);
        return not current_keys_[idx] and previous_keys_[idx];
    }
    [[nodiscard]] static constexpr auto mouse_position() noexcept -> vec2 {
        return mouse_position_;
    }

  private:
    [[nodiscard]] static constexpr auto key_to_size_t(const key key) noexcept -> std::size_t {
        return static_cast<std::size_t>(std::to_underlying(key));
    }

    static inline constinit std::array<bool, GLFW_KEY_LAST + 1> current_keys_{};
    static inline constinit std::array<bool, GLFW_KEY_LAST + 1> previous_keys_{};
    static inline constinit vec2                                mouse_position_{};
};
}  // namespace rin