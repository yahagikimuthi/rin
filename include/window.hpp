#pragma once

#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "camera.hpp"
#include "glad/glad.h"

#include "GLFW/glfw3.h"
#include "input.hpp"
#include "others/error.hpp"
#include "others/type.hpp"
#include "renderer/renderer.hpp"
#include "shape.hpp"

namespace rin {
inline void GLAPIENTRY message_callback(
    [[maybe_unused]] GLenum      source,
    [[maybe_unused]] GLenum      type,
    [[maybe_unused]] GLuint      id,
    [[maybe_unused]] GLenum      severity,
    [[maybe_unused]] GLsizei     length,
    const GLchar*                message,
    [[maybe_unused]] const void* userParam
) {
    std::cerr << "[OpenGL Debug Message]: " << message << '\n';
}

class Window final {
    enum class ErrorCode : u8 {
        failed_to_GLFW_initialize,
        failed_to_create_window,
        failed_to_create_shader
    };

  public:
    [[nodiscard]] static auto create(
        const i32 width, const i32 height, std::string_view title = "No Title"
    ) noexcept -> std::expected<Window, Error> {
        // GLFWの初期化（何回呼び出しても安全）
        if (not static_cast<bool>(glfwInit()))
            return Error::create(Error::runtime, "Failed to GLFW initialize");

        // これから作る画面のメタ設定
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // デバッグ有効

        auto        str    = std::string{title};
        auto* const window = glfwCreateWindow(width, height, str.c_str(), nullptr, nullptr);

        if (window == nullptr) return Error::create(Error::runtime, "Failed to initialize window");

        glfwMakeContextCurrent(window);
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));  // NOLINT

        auto actual_width  = 0;
        auto actual_height = 0;
        glfwGetFramebufferSize(window, &actual_width, &actual_height);

        glViewport(0, 0, actual_width, actual_height);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        auto camera = Camera{static_cast<f32>(actual_width), static_cast<f32>(actual_height)};

        auto renderer_res = Renderer::create();
        if (not renderer_res)
            return Error::create(Error::runtime, "Failed to Create Renderer", renderer_res.error());

        return Window{window, camera, std::move(*renderer_res)};
    }
    Window(const Window&) noexcept                    = delete;
    auto operator=(const Window&) noexcept -> Window& = delete;

    Window(Window&& other) noexcept
        : camera_{other.camera_},
          window_{std::exchange(other.window_, nullptr)},
          renderer_{std::move(other.renderer_)} {
        ++window_cnt_;
    }
    auto operator=(Window&& other) noexcept -> Window& {
        if (this == &other) return *this;

        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
        }
        window_   = std::exchange(other.window_, nullptr);
        camera_   = other.camera_;
        renderer_ = std::move(other.renderer_);
        return *this;
    }
    ~Window() noexcept {
        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
        }

        if (--window_cnt_ == 0) glfwTerminate();
    }

    [[nodiscard]] auto is_open() const noexcept -> bool {
        return not static_cast<bool>(glfwWindowShouldClose(window_));
    }

    void poll_events() noexcept {
        glfwPollEvents();
        Input::update(window_);
    }

    void clear(
        const f32 r = 0.f, const f32 g = 0.f, const f32 b = 0.f, const f32 alpha = 1.f
    ) noexcept {
        glClearColor(r, g, b, alpha);
        glClear(GL_COLOR_BUFFER_BIT);
        renderer_.use();
    }

    void draw(const IShape auto& shape) noexcept { renderer_.draw(shape, camera_); }

    void camera_position(const f32 x, const f32 y) noexcept { camera_.position(x, y); }
    void camera_position(glm::vec2 position) noexcept { camera_.position(position); }

    void display() noexcept { glfwSwapBuffers(window_); }

  private:
    explicit Window(GLFWwindow* const window, const Camera& camera, Renderer renderer) noexcept
        : camera_{camera}, window_{window}, renderer_{std::move(renderer)} {
        ++window_cnt_;

        // デバッグ出力の有効化
        glEnable(GL_DEBUG_OUTPUT);

        // 同期出力の有効化（エラーが発生したコードの位置で即座にコールバックを発生させる）
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // コールバック関数の登録
        glDebugMessageCallback(message_callback, nullptr);
    }

    Camera                      camera_;
    GLFWwindow*                 window_;
    Renderer                    renderer_;
    static inline constinit int window_cnt_{};
};
}  // namespace rin