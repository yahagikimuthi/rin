#pragma once

#include <algorithm>
#include <array>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "rin/color.hpp"
#include "rin/detail/camera.hpp"
#include "rin/detail/fbo_manager.hpp"
#include "rin/detail/input.hpp"
#include "rin/detail/renderer.hpp"
#include "rin/error.hpp"
#include "rin/extent.hpp"
#include "rin/key.hpp"
#include "rin/mouse.hpp"
#include "rin/sprite.hpp"
#include "rin/text.hpp"
#include "rin/types.hpp"
#include "rin/vec2.hpp"
#include "rin/vertex_vector.hpp"

/**
 * @file window.hpp
 * @brief ウィンドウの作成、イベント処理、入力管理、描画ルーチンを提供するモジュール
 */

namespace rin {

/**
 * @class window
 * @brief ウィンドウの生成、グラフィックスコンテキスト管理、描画・入力制御を統括するクラス
 */
class window final {
  public:
    /**
     * @brief 幅・高さを個別指定してウィンドウおよびコンテキストの初期化を試みます。
     *
     * @details GLFW の初期化、コンテキスト作成、GLAD による OpenGL 関数のロード、
     * 内部レンダラーおよび FBO (Frame Buffer Object) の生成を順次行います。
     *
     * @param width ウィンドウの幅（ピクセル）
     * @param height ウィンドウの高さ（ピクセル）
     * @param title ウィンドウのタイトル文字列
     * @param vsync 垂直同期（V-Sync）の有効化フラグ（デフォルト: true）
     * @return std::expected<window, error> 成功時は window インスタンス、初期化失敗時はエラー情報
     */
    [[nodiscard]] static auto try_make(
        const f32                   width,
        const f32                   height,
        const std::string_view      title,
        [[maybe_unused]] const bool vsync = true
    ) noexcept -> std::expected<window, error> {
        // GLFWの初期化（何回呼び出しても安全）
        if (not static_cast<bool>(glfwInit()))
            return make_error(
                runtime_error, "Failed to GLFW initialize. We recommend ending program."
            );

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);

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
        glfwSwapInterval(0);
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

    /**
     * @brief extent（サイズ構造体）を指定してウィンドウおよびコンテキストの初期化を試みます。
     *
     * @param size ウィンドウの幅と高さ（extent）
     * @param title ウィンドウのタイトル文字列（デフォルト: "No Title"）
     * @param vsync 垂直同期（V-Sync）の有効化フラグ（デフォルト: true）
     * @return std::expected<window, error> 成功時は window インスタンス、初期化失敗時はエラー情報
     */
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

    /**
     * @brief カメラの位置座標を取得します。
     * @return vec2 カメラの位置座標
     */
    [[nodiscard]] auto camera_position() const noexcept -> vec2 { return camera_.position(); }

    /**
     * @brief カメラの位置座標（X, Y成分）を設定します。
     * @param x X座標
     * @param y Y座標
     */
    void camera_position(const f32 x, const f32 y) noexcept { camera_.position(x, y); }

    /**
     * @brief カメラの位置座標を設定します。
     * @param position 位置座標
     */
    void camera_position(const vec2 position) noexcept { camera_.position(position); }

    /**
     * @brief カメラのズーム倍率を設定します。
     * @param zoom ズーム倍率
     */
    void camera_zoom(const f32 zoom) noexcept { camera_.zoom(zoom); }

    /**
     * @brief カメラのズーム倍率を取得します。
     * @return f32 ズーム倍率
     */
    [[nodiscard]] auto camera_zoom() const noexcept -> f32 { return camera_.zoom(); }

    /**
     * @brief 指定したキーが押されている状態（ホールド）かどうかを判定します。
     * @param button 対象のキーコード
     * @return true 押されている状態、false 押されていない状態
     */
    [[nodiscard]] auto is_key_down(const key button) const noexcept -> bool {
        return input_.is_key_down(button);
    }

    /**
     * @brief 指定したキーがこのフレームで押された瞬間かどうかを判定します。
     * @param button 対象のキーコード
     * @return true 押された瞬間、false それ以外
     */
    [[nodiscard]] auto is_key_pressed(const key button) const noexcept -> bool {
        return input_.is_key_pressed(button);
    }

    /**
     * @brief 指定したキーがこのフレームで離された瞬間かどうかを判定します。
     * @param button 対象のキーコード
     * @return true 離された瞬間、false それ以外
     */
    [[nodiscard]] auto is_key_released(const key button) const noexcept -> bool {
        return input_.is_key_released(button);
    }

    /**
     * @brief 指定したマウスボタンが押されている状態（ホールド）かどうかを判定します。
     * @param button 対象のマウスボタン
     * @return true 押されている状態、false 押されていない状態
     */
    [[nodiscard]] auto is_mouse_down(const mouse button) const noexcept -> bool {
        return input_.is_mouse_down(button);
    }

    /**
     * @brief 指定したマウスボタンがこのフレームで押された瞬間かどうかを判定します。
     * @param button 対象のマウスボタン
     * @return true 押された瞬間、false それ以外
     */
    [[nodiscard]] auto is_mouse_pressed(const mouse button) const noexcept -> bool {
        return input_.is_mouse_pressed(button);
    }

    /**
     * @brief 指定したマウスボタンがこのフレームで離された瞬間かどうかを判定します。
     * @param button 対象のマウスボタン
     * @return true 離された瞬間、false それ以外
     */
    [[nodiscard]] auto is_mouse_released(const mouse button) const noexcept -> bool {
        return input_.is_mouse_released(button);
    }

    /**
     * @brief 仮想画面座標系における現在のマウスカーソル位置を取得します。
     * @return vec2 マウスカーソルの位置座標
     */
    [[nodiscard]] auto mouse_position() const noexcept -> vec2 {
        return input_.mouse_position(vp_, virtual_size_);
    }

    [[nodiscard]] auto is_open() const noexcept -> bool {
        return not static_cast<bool>(glfwWindowShouldClose(window_));
    }

    /**
     * @brief イベントのポーリングを行い、内部の入力状態を更新します。
     *
     * @details フレームの開始時などに呼び出し、GLFW
     * のイベント処理と入力デバイスの状態更新を行います。
     */
    void poll_events() noexcept {
        glfwPollEvents();
        input_.update();
    }

    /**
     * @brief 描画処理を開始し、クリアカラーでオフスクリーンフレームバッファ（FBO）をクリアします。
     *
     * @param clear_color 画面消去時の背景色（デフォルト: black）
     */
    void begin_render(const rgba& clear_color = black) noexcept {
        const auto r = static_cast<f32>(clear_color.r) / 255.f;
        const auto g = static_cast<f32>(clear_color.g) / 255.f;
        const auto b = static_cast<f32>(clear_color.b) / 255.f;
        const auto a = static_cast<f32>(clear_color.a) / 255.f;
        fbo_manager_.bind(virtual_size_, r, g, b, a);
        renderer_.use();
    }

    /**
     * @brief RGBA値を個別に指定して描画処理を開始します。
     *
     * @param r 赤成分 (0〜255)
     * @param g 緑成分 (0〜255)
     * @param b 青成分 (0〜255)
     * @param a アルファ成分 (0〜255、デフォルト値: 255)
     */
    void begin_render(const u8 r, const u8 g, const u8 b, const u8 a = 255) noexcept {
        begin_render(rgba{.r = r, .g = g, .b = b, .a = a});
    }

    /**
     * @brief
     * 描画処理を終了し、オフスクリーンバッファの内容を画面（デフォルトフレームバッファ）へレンダリングしてバッファをスワップします。
     */
    void end_render() noexcept {
        fbo_manager_.unbind();

        glViewport(0, 0, static_cast<i32>(size_.width), static_cast<i32>(size_.height));

        constexpr auto black_arr = std::array<f32, 4>{0.f, 0.f, 0.f, 1.f};
        glClearNamedFramebufferfv(0, GL_COLOR, 0, black_arr.data());

        glViewport(vp_.x, vp_.y, vp_.w, vp_.h);

        fbo_manager_.render();
        glfwSwapBuffers(window_);
    }

    /**
     * @brief 頂点配列（vertex_vector）を描画します。
     * @param vec 描画対象の頂点コンテナ
     */
    void draw(const vertex_vector& vec) noexcept { renderer_.draw(vec, camera_, virtual_size_); }

    /**
     * @brief スプライトオブジェクトを描画します。
     * @param sprite_obj 描画対象のスプライト
     */
    void draw(sprite& sprite_obj) noexcept { renderer_.draw(sprite_obj, camera_, virtual_size_); }

    /**
     * @brief テキストオブジェクトを描画します。
     * @param tex 描画対象のテキスト
     */
    void draw(text& tex) noexcept { renderer_.draw(tex, camera_, virtual_size_); }

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
        glfwSetKeyCallback(
            window_,
            [](GLFWwindow*          win,
               i32                  keycode,
               [[maybe_unused]] i32 scancode,
               i32                  action,
               [[maybe_unused]] i32 mods) noexcept -> void {
                auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
                if (self == nullptr) return;
                self->input_.key_callback(keycode, action);
            }
        );
        glfwSetCursorPosCallback(window_, [](GLFWwindow* win, f64 x, f64 y) noexcept -> void {
            auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
            if (self == nullptr) return;
            self->input_.cursor_callback(static_cast<f32>(x), static_cast<f32>(y));
        });
        glfwSetMouseButtonCallback(
            window_,
            [](
                GLFWwindow* win, i32 button, i32 action, [[maybe_unused]] i32 mods
            ) noexcept -> void {
                auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
                if (self == nullptr) return;
                self->input_.mouse_button_callback(button, action);
            }
        );

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
    detail::view_point  vp_{};
    extent              size_;
    extent              virtual_size_;
};

/**
 * @brief 幅・高さを個別指定してウィンドウおよびコンテキストの初期化を試みるヘルパー関数
 *
 * @param width ウィンドウの幅（ピクセル）
 * @param height ウィンドウの高さ（ピクセル）
 * @param title ウィンドウのタイトル文字列（デフォルト: "No Title"）
 * @param vsync 垂直同期（V-Sync）の有効化フラグ（デフォルト: false）
 * @return std::expected<window, error> 成功時は window インスタンス、初期化失敗時はエラー情報
 */
[[nodiscard]] inline auto try_make_window(
    const f32 width, const f32 height, std::string_view title = "No Title", const bool vsync = false
) noexcept -> std::expected<window, error> {
    return window::try_make(width, height, title, vsync);
}

/**
 * @brief extent（サイズ構造体）を指定してウィンドウおよびコンテキストの初期化を試みるヘルパー関数
 *
 * @param size ウィンドウの幅と高さ（extent）
 * @param title ウィンドウのタイトル文字列（デフォルト: "No Title"）
 * @param vsync 垂直同期（V-Sync）の有効化フラグ（デフォルト: false）
 * @return std::expected<window, error> 成功時は window インスタンス、初期化失敗時はエラー情報
 */
[[nodiscard]] inline auto try_make_window(
    const extent size, std::string_view title = "No Title", const bool vsync = false
) noexcept -> std::expected<window, error> {
    return window::try_make(size, title, vsync);
}
}  // namespace rin