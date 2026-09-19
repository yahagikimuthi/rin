#pragma once

#include "glad/glad.h"

#include <expected>
#include <memory>
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

        auto* const window_ptr = glfwCreateWindow(
            static_cast<i32>(config.width),
            static_cast<i32>(config.height),
            config.title.data(),
            nullptr,
            nullptr
        );
        if (window_ptr == nullptr) {
            glfwTerminate();
            return std::unexpected{ErrorCode::failed_to_create_GLFW_window};
        }

        glfwMakeContextCurrent(window_ptr);

        if (not static_cast<bool>(
                gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))  // NOLINT
            )) {
            glfwDestroyWindow(window_ptr);
            glfwTerminate();
            return std::unexpected{ErrorCode::failed_to_initialize_GLAD};
        }

        return Window{*window_ptr, config.width, config.height, config.vsync};
    }

    [[nodiscard]] static auto create(const u32 width, const u32 height, std::string title)
        -> std::expected<Window, ErrorCode> {
        return create(WindowConfig{.width = width, .height = height, .title = std::move(title)});
    }
    [[nodiscard]] static auto create(const u32 width, const u32 height)
        -> std::expected<Window, ErrorCode> {
        return create(WindowConfig{.width = width, .height = height});
    }

    Window(const Window&) = delete;
    Window(Window&& other) noexcept
        : window_{other.window_}, width_{other.width_}, height_{other.height_} {
        other.window_ = nullptr;
    }
    auto operator=(const Window&) noexcept = delete;
    auto operator=(Window&& other) noexcept -> Window& {
        if (this == &other) return *this;

        if (window_ != nullptr) {
            glfwDestroyWindow(std::addressof(*window_));
        }
        window_       = other.window_;
        width_        = other.width_;
        height_       = other.height_;
        other.window_ = nullptr;
        return *this;
    }
    ~Window() noexcept {
        if (window_ != nullptr) {
            glfwDestroyWindow(std::addressof(*window_));
        }
        if (glfw_initialized_) {
            glfwTerminate();
            glfw_initialized_ = false;
        }
    }

    [[nodiscard]] auto is_open() const noexcept -> bool {
        return static_cast<bool>(glfwWindowShouldClose(window_));
    }
    void poll_events() const noexcept { glfwSwapBuffers(window_); }
    void swap_buffers() const noexcept { glfwSwapBuffers(window_); }

    static void clear(f32 r, f32 g, f32 b, f32 a) noexcept {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

  private:
    explicit Window(
        GLFWwindow& window, const u32 width, const u32 height, const bool vsync
    ) noexcept
        : window_{std::addressof(window)}, width_{width}, height_{height} {
        glfwSwapInterval(vsync ? 1 : 0);
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(
            window_, [](GLFWwindow* const win, const int w, const int h) noexcept -> void {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (not self) return;
                self->width_  = static_cast<u32>(w);
                self->height_ = static_cast<u32>(h);
                glViewport(0, 0, w, h);
            }
        );
    }

    GLFWwindow*                  window_{nullptr};
    u32                          width_;
    u32                          height_;
    static constinit inline bool glfw_initialized_{false};
};
}  // namespace rin