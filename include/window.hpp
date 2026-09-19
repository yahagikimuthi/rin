#pragma once

#include "glad/glad.h"

#include <expected>
#include <string>
#include <utility>

#include <GLFW/glfw3.h>

#include "type.hpp"

namespace rin {
struct WindowConfig final {
    u32         width{800};
    u32         height{600};
    std::string title{"No Title"};
    bool        vsync{true};
};

class Window final {
  public:
    enum class ErrorCode : u8 {
        failed_to_initialize_GLFW,
        failed_to_create_GLFW_window,
        failed_to_initialize_GLAD
    };

    [[nodiscard]] static auto create(const WindowConfig& config) noexcept
        -> std::expected<Window, ErrorCode> {
        if (not glfw_initialized_) {
            if (not static_cast<bool>(glfwInit())) {
                return std::unexpected{ErrorCode::failed_to_initialize_GLFW};
            }
            glfw_initialized_ = true;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // std::string_view を null 終端文字列へ安全に渡すため std::string を使用
        const std::string title_str{config.title};
        auto* const       window_ptr = glfwCreateWindow(
            static_cast<i32>(config.width),
            static_cast<i32>(config.height),
            title_str.c_str(),
            nullptr,
            nullptr
        );

        if (window_ptr == nullptr) {
            return std::unexpected{ErrorCode::failed_to_create_GLFW_window};
        }

        glfwMakeContextCurrent(window_ptr);

        if (not static_cast<bool>(
                gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))  // NOLINT
            )) {
            glfwDestroyWindow(window_ptr);
            return std::unexpected{ErrorCode::failed_to_initialize_GLAD};
        }

        return Window{window_ptr, config.width, config.height, config.vsync};
    }

    [[nodiscard]] static auto create(const u32 width, const u32 height, std::string title) noexcept
        -> std::expected<Window, ErrorCode> {
        return create(WindowConfig{.width = width, .height = height, .title = std::move(title)});
    }

    [[nodiscard]] static auto create(const u32 width, const u32 height) noexcept
        -> std::expected<Window, ErrorCode> {
        return create(WindowConfig{.width = width, .height = height});
    }

    // コピー禁止
    Window(const Window&)                             = delete;
    auto operator=(const Window&) noexcept -> Window& = delete;

    // ムーブ構築
    Window(Window&& other) noexcept
        : window_{other.window_}, width_{other.width_}, height_{other.height_} {
        other.window_ = nullptr;
        if (window_ != nullptr) {
            glfwSetWindowUserPointer(window_, this);
        }
    }

    // ムーブ代入
    auto operator=(Window&& other) noexcept -> Window& {
        if (this == &other) return *this;

        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
        }

        window_       = other.window_;
        width_        = other.width_;
        height_       = other.height_;
        other.window_ = nullptr;

        if (window_ != nullptr) {
            glfwSetWindowUserPointer(window_, this);
        }

        return *this;
    }

    ~Window() noexcept {
        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
    }

    // ウィンドウが開いているか判定（glfwWindowShouldClose は閉じる要求が出たら true を返す）
    [[nodiscard]] auto is_open() const noexcept -> bool {
        if (window_ == nullptr) return false;
        return not static_cast<bool>(glfwWindowShouldClose(window_));
    }

    static void poll_events() noexcept { glfwPollEvents(); }

    void swap_buffers() const noexcept {
        if (window_ != nullptr) {
            glfwSwapBuffers(window_);
        }
    }

    static void clear(
        const f32 r = 0.1f, const f32 g = 0.12f, const f32 b = 0.15f, const f32 a = 1.0f
    ) noexcept {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    [[nodiscard]] auto get_native_handle() const noexcept -> GLFWwindow* { return window_; }
    [[nodiscard]] auto get_width() const noexcept -> u32 { return width_; }
    [[nodiscard]] auto get_height() const noexcept -> u32 { return height_; }

  private:
    explicit Window(
        GLFWwindow* const window, const u32 width, const u32 height, const bool vsync
    ) noexcept
        : window_{window}, width_{width}, height_{height} {
        glfwSwapInterval(vsync ? 1 : 0);
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(
            window_, [](GLFWwindow* const win, const int w, const int h) noexcept -> void {
                auto* const self = static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (self == nullptr) return;
                self->width_  = static_cast<u32>(w);
                self->height_ = static_cast<u32>(h);
                glViewport(0, 0, w, h);
            }
        );
    }

    GLFWwindow*                  window_{nullptr};
    u32                          width_{0};
    u32                          height_{0};
    static constinit inline bool glfw_initialized_{false};
};
}  // namespace rin