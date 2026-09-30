#pragma once

#include <algorithm>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "detail/camera.hpp"
#include "detail/fbo_manager.hpp"
#include "detail/input.hpp"
#include "detail/renderer.hpp"
#include "error.hpp"
#include "extent.hpp"
#include "key.hpp"
#include "mouse.hpp"
#include "sprite.hpp"
#include "text.hpp"
#include "types.hpp"
#include "vec2.hpp"
#include "vertex_vector.hpp"

namespace rin {
inline void GLAPIENTRY message_callback(
    [[maybe_unused]] GLenum      source,
    [[maybe_unused]] GLenum      type,
    [[maybe_unused]] GLuint      id,
    [[maybe_unused]] GLenum      severity,
    [[maybe_unused]] GLsizei     length,
    const GLchar*                message,
    [[maybe_unused]] const void* userParam
) noexcept {
    std::cerr << "[OpenGL Debug Message]: " << message << '\n';
}

class window final {
    struct view_point final {
        i32 x;
        i32 y;
        i32 w;
        i32 h;
    };

  public:
    [[nodiscard]] static auto try_make(
        const f32 width, const f32 height, const std::string_view title, const bool vsync = true
    ) noexcept -> std::expected<window, error> {
        // GLFWの初期化（何回呼び出しても安全）
        if (not static_cast<bool>(glfwInit()))
            return make_error(
                runtime_error, "Failed to GLFW initialize. We recommend ending program."
            );

        // これから作る画面のメタ設定
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // デバッグ有効

        auto        str        = std::string{title};
        auto* const window_ptr = glfwCreateWindow(
            static_cast<i32>(std::max(width, 0.f)),
            static_cast<i32>(std::max(height, 1.f)),
            str.c_str(),
            nullptr,
            nullptr
        );

        if (window_ptr == nullptr) return make_error(runtime_error, "Failed to initialize window.");

        glfwMakeContextCurrent(window_ptr);
        glfwSwapInterval(static_cast<i32>(vsync));
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));  // NOLINT

        auto actual_width  = 0;
        auto actual_height = 0;
        glfwGetFramebufferSize(window_ptr, &actual_width, &actual_height);

        glViewport(0, 0, actual_width, actual_height);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        auto renderer_res = detail::renderer::try_make();
        if (not renderer_res)
            return make_error(runtime_error, "Failed to create renderer.", renderer_res.error());

        auto fbo = detail::fbo_manager::try_make({.width = width, .height = height});
        if (not fbo) return make_error(runtime_error, "Failed to create FBO.", fbo.error());

        return window{
            window_ptr,
            std::move(*renderer_res),
            std::move(*fbo),
            {.width = static_cast<f32>(actual_width), .height = static_cast<f32>(actual_height)},
            {.width = width, .height = height}
        };
    }

    [[nodiscard]] static auto try_make(
        const extent size, const std::string_view title = "No Title", const bool vsync = true
    ) noexcept -> std::expected<window, error> {
        return try_make(size.width, size.height, title, vsync);
    }

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : input_{other.input_},
          camera_{other.camera_},
          window_{std::exchange(other.window_, nullptr)},
          renderer_{std::move(other.renderer_)},
          fbo_manager_{std::move(other.fbo_manager_)},
          vp_{other.vp_},
          size_{other.size_},
          virtual_size_{other.virtual_size_} {
        glfwSetWindowUserPointer(window_, this);
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        input_        = other.input_;
        window_       = std::exchange(other.window_, nullptr);
        camera_       = other.camera_;
        renderer_     = std::move(other.renderer_);
        size_         = other.size_;
        virtual_size_ = other.virtual_size_;
        vp_           = other.vp_;

        glfwSetWindowUserPointer(window_, this);
        return *this;
    }

    ~window() noexcept {
        if (window_ == nullptr) return;
        glfwSetWindowUserPointer(window_, nullptr);
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    [[nodiscard]] auto camera_position() const noexcept -> vec2 { return camera_.position(); }

    void camera_position(const f32 x, const f32 y) noexcept { camera_.position(x, y); }
    void camera_position(const vec2 position) noexcept { camera_.position(position); }
    void camera_zoom(const f32 zoom) noexcept { camera_.zoom(zoom); }

    [[nodiscard]] auto camera_zoom() const noexcept -> f32 { return camera_.zoom(); }

    [[nodiscard]] auto is_key_down(const key button) const noexcept -> bool {
        return input_.is_key_down(button);
    }

    [[nodiscard]] auto is_key_pressed(const key button) const noexcept -> bool {
        return input_.is_key_pressed(button);
    }

    [[nodiscard]] auto is_key_released(const key button) const noexcept -> bool {
        return input_.is_key_released(button);
    }

    [[nodiscard]] auto is_mouse_down(const mouse button) const noexcept -> bool {
        return input_.is_mouse_down(button);
    }

    [[nodiscard]] auto is_mouse_pressed(const mouse button) const noexcept -> bool {
        return input_.is_mouse_pressed(button);
    }

    [[nodiscard]] auto is_mouse_released(const mouse button) const noexcept -> bool {
        return input_.is_mouse_released(button);
    }

    [[nodiscard]] auto mouse_position() const noexcept -> vec2 {
        return input_.mouse_position(size_, virtual_size_);
    }

    [[nodiscard]] auto is_open() const noexcept -> bool {
        return not static_cast<bool>(glfwWindowShouldClose(window_));
    }

    void poll_events() noexcept {
        glfwPollEvents();
        input_.update(window_);
    }

    void begin_render() noexcept {
        fbo_manager_.bind(virtual_size_);

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer_.use();
    }

    void end_render() noexcept {
        fbo_manager_.bind_for_bit();

        glViewport(0, 0, static_cast<i32>(size_.width), static_cast<i32>(size_.height));
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glDrawBuffer(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT);

        glBlitFramebuffer(
            0,
            0,
            static_cast<i32>(virtual_size_.width),
            static_cast<i32>(virtual_size_.height),
            vp_.x,
            vp_.y,
            vp_.x + vp_.w,
            vp_.y + vp_.h,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );

        fbo_manager_.unbind();

        glfwSwapBuffers(window_);
    }

    void draw(const vertex_vector& vec) noexcept { renderer_.draw(vec, camera_, virtual_size_); }

    void draw(sprite& sprite_obj) noexcept { renderer_.draw(sprite_obj, camera_, virtual_size_); }

    void draw(text& tex) noexcept { renderer_.draw(tex, camera_, virtual_size_); }

    [[nodiscard]] auto native_window() noexcept -> GLFWwindow* { return window_; }

  private:
    explicit window(
        GLFWwindow* const   window_ptr,
        detail::renderer    renderer_object,
        detail::fbo_manager fbo,
        const extent&       size,
        const extent&       virtual_size
    ) noexcept
        : window_{window_ptr},
          renderer_{std::move(renderer_object)},
          fbo_manager_{std::move(fbo)},
          size_{size},
          virtual_size_{virtual_size} {
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, window_size_callback);

        glEnable(GL_CULL_FACE);

        window_size_callback(window_, static_cast<i32>(size.width), static_cast<i32>(size.height));
    }

    static void window_size_callback(GLFWwindow* win, i32 fb_w, i32 fb_h) noexcept {
        auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
        if (self == nullptr || fb_w <= 0 || fb_h <= 0) return;

        // 仮想サイズとのアスペクト比計算
        const auto target_aspect = self->virtual_size_.width / self->virtual_size_.height;
        const auto fb_aspect     = static_cast<f32>(fb_w) / static_cast<f32>(fb_h);

        auto vp_w = static_cast<f32>(fb_w);
        auto vp_h = static_cast<f32>(fb_h);
        auto vp_x = 0.f;
        auto vp_y = 0.f;

        if (fb_aspect > target_aspect) {
            vp_w = static_cast<f32>(fb_h) * target_aspect;
            vp_x = (static_cast<f32>(fb_w) - vp_w) * 0.5f;
        } else {
            vp_h = static_cast<f32>(fb_w) / target_aspect;
            vp_y = (static_cast<f32>(fb_h) - vp_h) * 0.5f;
        }

        self->vp_ = {
            .x = static_cast<i32>(vp_x),
            .y = static_cast<i32>(vp_y),
            .w = static_cast<i32>(vp_w),
            .h = static_cast<i32>(vp_h)
        };

        self->size_ = extent{.width = static_cast<f32>(fb_w), .height = static_cast<f32>(fb_h)};
    }

    detail::input       input_;
    detail::camera      camera_;
    GLFWwindow*         window_;  // 所有権を持たない
    detail::renderer    renderer_;
    detail::fbo_manager fbo_manager_;
    view_point          vp_{};
    extent              size_;
    extent              virtual_size_;
};

[[nodiscard]] inline auto try_make_window(
    const f32 width, const f32 height, std::string_view title = "No Title", const bool vsync = true
) noexcept -> std::expected<window, error> {
    return window::try_make(width, height, title, vsync);
}

[[nodiscard]] inline auto try_make_window(
    const extent size, std::string_view title = "No Title", const bool vsync = true
) noexcept -> std::expected<window, error> {
    return window::try_make(size, title, vsync);
}
}  // namespace rin