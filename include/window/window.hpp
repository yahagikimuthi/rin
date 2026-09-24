#pragma once

#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "camera.hpp"
#include "glad/glad.h"

#include "GLFW/glfw3.h"
#include "others/error.hpp"
#include "others/type.hpp"
#include "others/util.hpp"
#include "renderer/renderer.hpp"
#include "texture/sprite.hpp"
#include "texture/text.hpp"
#include "vertex.hpp"
#include "window/input.hpp"

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
    [[nodiscard]] static auto create(
        const extent size, std::string_view title = "No Title"
    ) noexcept -> std::expected<window, error> {
        return create(static_cast<i32>(size.width), static_cast<i32>(size.height), title);
    }

    [[nodiscard]] static auto create(
        const i32 width, const i32 height, std::string_view title = "No Title"
    ) noexcept -> std::expected<window, error> {
        // GLFWの初期化（何回呼び出しても安全）
        if (not static_cast<bool>(glfwInit()))
            return error::create(
                runtime_error, "Failed to GLFW initialize. We recommend ending program."
            );

        // これから作る画面のメタ設定
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // デバッグ有効

        auto        str        = std::string{title};
        auto* const window_ptr = glfwCreateWindow(width, height, str.c_str(), nullptr, nullptr);

        if (window_ptr == nullptr)
            return error::create(runtime_error, "Failed to initialize window.");

        glfwMakeContextCurrent(window_ptr);
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));  // NOLINT

        auto actual_width  = 0;
        auto actual_height = 0;
        glfwGetFramebufferSize(window_ptr, &actual_width, &actual_height);

        glViewport(0, 0, actual_width, actual_height);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        auto camera_res = camera{static_cast<f32>(actual_width), static_cast<f32>(actual_height)};

        auto renderer_res = renderer::create();
        if (not renderer_res)
            return error::create(runtime_error, "Failed to create renderer.", renderer_res.error());

        return window{window_ptr, camera_res, std::move(*renderer_res)};
    }
    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : camera_{other.camera_},
          window_{std::exchange(other.window_, nullptr)},
          renderer_{std::move(other.renderer_)} {}
    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        window_   = std::exchange(other.window_, nullptr);
        camera_   = other.camera_;
        renderer_ = std::move(other.renderer_);
        return *this;
    }
    ~window() noexcept {
        if (window_ != nullptr) {
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
        input::update(window_);
        renderer_.use();
    }

    static void clear(
        const f32 r = 0.f, const f32 g = 0.f, const f32 b = 0.f, const f32 a = 1.f
    ) noexcept {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void draw(const vertex_vector& vec) noexcept { renderer_.draw(vec, camera_); }

    void draw(sprite& sprite_obj) noexcept { renderer_.draw(sprite_obj, camera_); }

    void draw(const text& tex) noexcept { renderer_.draw(tex, camera_); }

    [[nodiscard]] auto camera_position() const noexcept -> vec2 { return camera_.position(); }

    void camera_position(const f32 x, const f32 y) noexcept { camera_.position(x, y); }
    void camera_position(const vec2 position) noexcept { camera_.position(position); }

    void display() noexcept { glfwSwapBuffers(window_); }

  private:
    explicit window(
        GLFWwindow* const window_ptr, const camera& camera_object, renderer renderer_object
    ) noexcept
        : camera_{camera_object}, window_{window_ptr}, renderer_{std::move(renderer_object)} {
        ++window_cnt_;

        // デバッグ出力の有効化
        glEnable(GL_DEBUG_OUTPUT);

        // 同期出力の有効化（エラーが発生したコードの位置で即座にコールバックを発生させる）
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // コールバック関数の登録
        glDebugMessageCallback(message_callback, nullptr);
    }

    camera                      camera_;
    GLFWwindow*                 window_;
    renderer                    renderer_;
    static inline constinit int window_cnt_{};
};
}  // namespace rin