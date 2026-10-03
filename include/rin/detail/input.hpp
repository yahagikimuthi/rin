#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <glm/ext/vector_float2.hpp>

#include "rin/detail/graphics.hpp"
#include "rin/extent.hpp"
#include "rin/key.hpp"
#include "rin/mouse.hpp"
#include "rin/types.hpp"
#include "rin/vec2.hpp"

namespace rin::detail {
struct view_point final {
    i32 x;
    i32 y;
    i32 w;
    i32 h;
};

template <std::size_t N>
struct input_log final {
    std::array<bool, N> next;
    std::array<bool, N> current;
    std::array<bool, N> previous;
};

class key_input final {
  public:
    explicit key_input() noexcept = default;

    void update() noexcept {
        key_.previous = key_.current;
        key_.current  = key_.next;
    }

    void callback(const i32 code, const i32 action) noexcept {
        if (code < 0 or code >= static_cast<i32>(GLFW_KEY_LAST)) return;

        const auto idx = static_cast<std::size_t>(code);
        if (action == GLFW_PRESS) {
            key_.next[idx] = true;
        } else if (action == GLFW_RELEASE) {
            key_.next[idx] = false;
        }
    }

    [[nodiscard]] auto is_down(const key button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return key_.current[idx];
    }

    [[nodiscard]] auto is_pressed(const key button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return key_.current[idx] and not key_.previous[idx];
    }

    [[nodiscard]] auto is_released(const key button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return not key_.current[idx] and key_.previous[idx];
    }

  private:
    [[nodiscard]] static auto to_size_t(const key button) noexcept -> std::size_t {
        return static_cast<std::size_t>(button);
    }

    input_log<GLFW_KEY_LAST> key_{};
};

class mouse_input final {
  public:
    explicit mouse_input() noexcept = default;

    void update() noexcept {
        button_.previous = button_.current;
        button_.current  = button_.next;

        scroll_.current = scroll_.next;
        scroll_.next.fill(false);
    }

    void button_callback(const i32 button, const i32 action) noexcept {
        if (button < 0 or button > GLFW_MOUSE_BUTTON_LAST) return;

        const auto idx = static_cast<std::size_t>(button);
        if (action == GLFW_PRESS) {
            button_.next[idx] = true;
        } else if (action == GLFW_RELEASE) {
            button_.next[idx] = false;
        }
    }

    void cursor_callback(const f32 x, const f32 y) noexcept { position_ = {.x = x, .y = y}; }

    void scroll_callback(const scroll action) noexcept {
        auto idx          = static_cast<std::size_t>(action);
        scroll_.next[idx] = true;
    }

    [[nodiscard]] auto is_scroll(const scroll action) const noexcept -> bool {
        return scroll_.current[static_cast<std::size_t>(action)];
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
        return button_.current[idx];
    }

    [[nodiscard]] auto is_pressed(const mouse button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return button_.current[idx] and not button_.previous[idx];
    }

    [[nodiscard]] auto is_released(const mouse button) const noexcept -> bool {
        const auto idx = to_size_t(button);
        return not button_.current[idx] and button_.previous[idx];
    }

  private:
    [[nodiscard]] static auto to_size_t(const mouse button) noexcept -> std::size_t {
        return static_cast<std::size_t>(button);
    }

    input_log<GLFW_MOUSE_BUTTON_LAST>                  button_{};
    input_log<static_cast<std::size_t>(scroll::count)> scroll_{};
    vec2                                               position_;
};

class input final {
  public:
    explicit input() noexcept = default;

    void update() noexcept {
        key_.update();
        mouse_.update();
    }

    void key_callback(const i32 code, const i32 action) noexcept { key_.callback(code, action); }

    void cursor_callback(const f32 x, const f32 y) noexcept { mouse_.cursor_callback(x, y); }

    void mouse_button_callback(const i32 button, const i32 action) noexcept {
        mouse_.button_callback(button, action);
    }

    void scroll_callback(const scroll action) noexcept { mouse_.scroll_callback(action); }

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

    [[nodiscard]] auto is_scroll(const scroll action) const noexcept -> bool {
        return mouse_.is_scroll(action);
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