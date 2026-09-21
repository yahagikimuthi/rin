#pragma once

#include <GLFW/glfw3.h>
#include <array>
#include <cstddef>
#include <glm/ext/vector_float2.hpp>
#include <ranges>
#include <utility>

#include "type.hpp"

namespace rin {
enum class Key : u8 {
    a     = GLFW_KEY_A,
    b     = GLFW_KEY_B,
    c     = GLFW_KEY_C,
    d     = GLFW_KEY_D,
    e     = GLFW_KEY_E,
    f     = GLFW_KEY_F,
    g     = GLFW_KEY_G,
    h     = GLFW_KEY_H,
    i     = GLFW_KEY_I,
    j     = GLFW_KEY_J,
    k     = GLFW_KEY_K,
    l     = GLFW_KEY_L,
    m     = GLFW_KEY_M,
    n     = GLFW_KEY_N,
    o     = GLFW_KEY_O,
    p     = GLFW_KEY_P,
    q     = GLFW_KEY_Q,
    r     = GLFW_KEY_R,
    s     = GLFW_KEY_S,
    t     = GLFW_KEY_T,
    u     = GLFW_KEY_U,
    v     = GLFW_KEY_V,
    w     = GLFW_KEY_W,
    x     = GLFW_KEY_X,
    y     = GLFW_KEY_Y,
    z     = GLFW_KEY_Z,
    space = GLFW_KEY_SPACE
};

class Input final {
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
        mouse_position_ = {static_cast<f32>(x), static_cast<f32>(y)};
    }

    [[nodiscard]] static constexpr auto is_key_down(const Key key) noexcept -> bool {
        const auto idx = key_to_size_t(key);
        return current_keys_[idx];
    }
    [[nodiscard]] static constexpr auto is_key_pressed(const Key key) noexcept -> bool {
        const auto idx = key_to_size_t(key);
        return current_keys_[idx] and not previous_keys_[idx];
    }
    [[nodiscard]] static constexpr auto is_key_released(const Key key) noexcept -> bool {
        const auto idx = key_to_size_t(key);
        return not current_keys_[idx] and previous_keys_[idx];
    }
    [[nodiscard]] static constexpr auto mouse_position() noexcept -> glm::vec2 {
        return mouse_position_;
    }

  private:
    [[nodiscard]] static constexpr auto key_to_size_t(const Key key) noexcept -> std::size_t {
        return static_cast<std::size_t>(std::to_underlying(key));
    }

    static inline constinit std::array<bool, GLFW_KEY_LAST + 1> current_keys_{};
    static inline constinit std::array<bool, GLFW_KEY_LAST + 1> previous_keys_{};
    static inline constinit glm::vec2                           mouse_position_{0.f, 0.f};
};
}  // namespace rin