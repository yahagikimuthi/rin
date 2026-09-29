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
#include "type.hpp"
#include "vec2.hpp"

namespace rin::detail {
class key_input final {
  public:
    explicit key_input() noexcept = default;

    void update(GLFWwindow* win) noexcept {
        previous_ = current_;

        for (const auto i : std::views::indices(current_.size())) {
            current_[i] = (glfwGetKey(win, static_cast<i32>(i))) == GLFW_PRESS;
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
        const extent& window_size, const extent& virtual_window_size
    ) const noexcept -> vec2 {
        if (window_size.width <= 0.f || window_size.height <= 0.f) {
            return vec2{0.f, 0.f};
        }

        const auto target_aspect = virtual_window_size.width / virtual_window_size.height;
        const auto window_aspect = window_size.width / window_size.height;

        auto render_w = window_size.width;
        auto render_h = window_size.height;
        auto offset_x = 0.f;
        auto offset_y = 0.f;

        if (window_aspect > target_aspect) {
            // 左右に黒帯
            render_w = window_size.height * target_aspect;
            offset_x = (window_size.width - render_w) * 0.5f;
        } else {
            // 上下に黒帯
            render_h = window_size.width / target_aspect;
            offset_y = (window_size.height - render_h) * 0.5f;
        }

        // 1. 黒帯のオフセットを除外し、描画領域内での 0.0 ~ 1.0 に正規化
        const auto norm_x = (position_.x - offset_x) / render_w;

        // GLFWのマウス(yは上から下) を 左下原点(yは下から上) に反転させて正規化
        const auto norm_y = 1.f - ((position_.y - offset_y) / render_h);

        // 2. 仮想解像度にスケール
        const auto virt_x = norm_x * virtual_window_size.width;
        const auto virt_y = norm_y * virtual_window_size.height;

        // ※ 必要に応じて clamp (黒帯部分を押し出す場合はコメント解除)
        // virt_x = std::clamp(virt_x, 0.f, virtual_window_size.width);
        // virt_y = std::clamp(virt_y, 0.f, virtual_window_size.height);

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

        auto x = f64{};
        auto y = f64{};

        glfwGetCursorPos(win, &x, &y);
        mouse_position_ = {.x = static_cast<f32>(x), .y = static_cast<f32>(y)};
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
        const extent& window_size, const extent& virtual_window_size
    ) const noexcept -> vec2 {
        return mouse_.position(window_size, virtual_window_size);
    }

  private:
    key_input   key_;
    mouse_input mouse_;
    vec2        mouse_position_{};
};

}  // namespace rin::detail