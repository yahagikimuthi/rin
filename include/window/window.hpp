#pragma once

#include <algorithm>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "glad/glad.h"

#include "GLFW/glfw3.h"
#include "camera.hpp"
#include "key.hpp"
#include "others/error.hpp"
#include "others/type.hpp"
#include "others/util.hpp"
#include "renderer/renderer.hpp"
#include "texture/sprite.hpp"
#include "texture/text.hpp"
#include "vertex.hpp"
#include "window/input.hpp"
#include "window/mouse.hpp"

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
  public:
    [[nodiscard]] static auto try_make(
        const f32 width, const f32 height, std::string_view title
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
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));  // NOLINT

        auto actual_width  = 0;
        auto actual_height = 0;
        glfwGetFramebufferSize(window_ptr, &actual_width, &actual_height);

        glViewport(0, 0, actual_width, actual_height);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        auto camera_obj =
            make_camera(static_cast<f32>(actual_width), static_cast<f32>(actual_height));

        auto renderer_res = try_make_renderer();
        if (not renderer_res)
            return make_error(runtime_error, "Failed to create renderer.", renderer_res.error());

        return window{
            window_ptr,
            camera_obj,
            std::move(*renderer_res),
            {.width = static_cast<f32>(actual_width), .height = static_cast<f32>(actual_height)}
        };
    }

    [[nodiscard]] static auto try_make(
        const extent size, const std::string_view title = "No Title"
    ) noexcept -> std::expected<window, error> {
        return try_make(size.width, size.height, title);
    }

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : input_{other.input_},
          camera_{other.camera_},
          window_{std::exchange(other.window_, nullptr)},
          renderer_{std::move(other.renderer_)},
          size_{other.size_} {
        glfwSetWindowUserPointer(window_, this);
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        input_    = other.input_;
        window_   = std::exchange(other.window_, nullptr);
        camera_   = other.camera_;
        renderer_ = std::move(other.renderer_);
        size_     = other.size_;

        glfwSetWindowUserPointer(window_, this);
        return *this;
    }

    ~window() noexcept {
        if (window_ != nullptr) {
            glfwSetWindowUserPointer(window_, nullptr);
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        --window_cnt_;

        if (--window_cnt_ == 0) glfwTerminate();
    }

    [[nodiscard]] auto is_open() const noexcept -> bool {
        return not static_cast<bool>(glfwWindowShouldClose(window_));
    }

    void poll_events() noexcept {
        glfwPollEvents();
        input_.update(window_);
        renderer_.use();
    }

    static void clear(
        const f32 r = 0.f, const f32 g = 0.f, const f32 b = 0.f, const f32 a = 1.f
    ) noexcept {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    static void clear(const rgba& color) noexcept { clear(color.r, color.g, color.b, color.a); }

    void draw(const vertex_vector& vec) noexcept { renderer_.draw(vec, camera_); }

    void draw(sprite& sprite_obj) noexcept { renderer_.draw(sprite_obj, camera_); }

    void draw(const text& tex) noexcept { renderer_.draw(tex, camera_); }

    [[nodiscard]] auto camera_position() const noexcept -> vec2 { return camera_.position(); }

    void camera_position(const f32 x, const f32 y) noexcept { camera_.position(x, y); }
    void camera_position(const vec2 position) noexcept { camera_.position(position); }
    void camera_zoom(const f32 zoom) noexcept { camera_.zoom(zoom); }

    [[nodiscard]] auto camera_zoom() const noexcept -> f32 { return camera_.zoom(); }

    void display() noexcept { glfwSwapBuffers(window_); }

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
        return input_.mouse_position(size_);
    }

  private:
    explicit window(
        GLFWwindow* const window_ptr,
        const camera&     camera_object,
        renderer          renderer_object,
        const extent&     size
    ) noexcept
        : camera_{camera_object},
          window_{window_ptr},
          renderer_{std::move(renderer_object)},
          size_{size} {
        ++window_cnt_;

        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* win, i32 w, i32 h) noexcept -> void {
            auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
            if (self == nullptr) return;
            glViewport(0, 0, w, h);

            auto actual_width  = 0;
            auto actual_height = 0;
            glfwGetFramebufferSize(win, &actual_width, &actual_height);

            const auto casted_w = static_cast<f32>(actual_width);
            const auto casted_h = static_cast<f32>(actual_height);

            self->size_ = rin::extent{.width = casted_w, .height = casted_h};
            self->camera_.window_size(casted_w, casted_h);
        });

        // デバッグ出力の有効化
        glEnable(GL_DEBUG_OUTPUT);

        // 同期出力の有効化（エラーが発生したコードの位置で即座にコールバックを発生させる）
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // コールバック関数の登録
        glDebugMessageCallback(message_callback, nullptr);
    }

    input                       input_{make_input()};
    camera                      camera_;
    GLFWwindow*                 window_;
    renderer                    renderer_;
    extent                      size_;
    static inline constinit int window_cnt_{};
};

[[nodiscard]] inline auto try_make_window(
    const f32 width, const f32 height, std::string_view title = "No Title"
) noexcept -> std::expected<window, error> {
    return window::try_make(width, height, title);
}

[[nodiscard]] inline auto try_make_window(
    const extent size, std::string_view title = "No Title"
) noexcept -> std::expected<window, error> {
    return window::try_make(size, title);
}
}  // namespace rin