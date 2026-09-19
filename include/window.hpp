#pragma once

#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "glad/glad.h"

#include "GLFW/glfw3.h"
#include "type.hpp"

namespace rin {
struct WindowConfig final {
    u32              width{800};
    u32              height{600};
    std::string_view title{"No Title"};
    bool             vsync{true};
};

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
    enum class ErrorCode : u8 { failed_to_glfw_initialize, failed_to_create_window };

  public:
    [[nodiscard]] static auto create(
        const i32 width, const i32 height, std::string_view title
    ) noexcept -> std::expected<Window, ErrorCode> {
        // GLFWの初期化（何回呼び出しても安全）
        if (not static_cast<bool>(glfwInit()))
            return std::unexpected{ErrorCode::failed_to_glfw_initialize};

        // これから作る画面のメタ設定
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // デバッグ有効

        auto        str    = std::string{title};
        auto* const window = glfwCreateWindow(width, height, str.c_str(), nullptr, nullptr);

        if (window == nullptr) return std::unexpected{ErrorCode::failed_to_create_window};
        return Window{window, width, height};
    }
    Window(const Window&) noexcept                    = delete;
    auto operator=(const Window&) noexcept -> Window& = delete;

    Window(Window&& other) noexcept : window_{std::exchange(other.window_, nullptr)} {
        ++window_cnt_;
    }
    auto operator=(Window&& other) noexcept -> Window& {
        if (this == &other) return *this;

        if (window_ != nullptr) glfwDestroyWindow(window_);
        window_ = std::exchange(other.window_, nullptr);
        return *this;
    }
    ~Window() noexcept {
        if (window_ != nullptr) glfwDestroyWindow(window_);

        if (--window_cnt_ == 0) glfwTerminate();
    }

    [[nodiscard]] auto is_open() const noexcept -> bool {
        return not static_cast<bool>(glfwWindowShouldClose(window_));
    }

    static void begin_frame() noexcept {
        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void display() noexcept { glfwSwapBuffers(window_); }

  private:
    explicit Window(GLFWwindow* const window, const i32 width, const i32 height) noexcept
        : window_{window} {
        glfwMakeContextCurrent(window_);
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));  // NOLINT

        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        ++window_cnt_;

        // デバッグ出力の有効化
        glEnable(GL_DEBUG_OUTPUT);

        // 同期出力の有効化（エラーが発生したコードの位置で即座にコールバックを発生させる）
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // コールバック関数の登録
        glDebugMessageCallback(message_callback, nullptr);
    }

    GLFWwindow*                 window_;
    static inline constinit int window_cnt_{};
};
}  // namespace rin