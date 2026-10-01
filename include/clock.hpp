#pragma once

#include "detail/fps_counter.hpp"
#include "detail/graphics.hpp"
#include "types.hpp"

/**
 * @file clock.hpp
 * @brief 経過時間（デルタタイム）および FPS（フレームレート）の計測を行うモジュール
 */

namespace rin {
/**
 * @class tick
 * @brief フレーム間の経過時間計測および FPS 算出を行うタイマークラス
 *
 * @details GLFW の `glfwGetTime()` を用いて時間の計測を行います。
 * 毎フレーム `tick()` を呼ぶことでデルタタイムおよび FPS が更新されます。
 */
class clock final {
  public:
    /**
     * @brief tick インスタンスを生成します。
     * @return tick 生成された tick インスタンス
     */
    [[nodiscard]] static auto make() noexcept -> clock { return clock{}; }

    /**
     * @brief タイマーを更新し、現在のプログラム経過時間を返します。
     *
     * @details フレーム毎に本メソッドを呼び出すことで、内部のデルタタイムおよび FPS
     * カウンタが更新されます。
     * @return f32 現在の経過時間（秒単位）
     */
    auto tick() noexcept -> f32 {
        const auto current_time = glfwGetTime();
        delta_time_             = current_time - last_time_;
        last_time_              = current_time;
        fps_counter_.update();
        return static_cast<f32>(current_time);
    }

    /**
     * @brief 前回の `tick()` 呼び出しからの経過時間（デルタタイム）を取得します。
     * @return f32 デルタタイム（秒単位）
     */
    [[nodiscard]] constexpr auto delta_time() const noexcept -> f32 {
        return static_cast<f32>(delta_time_);
    }

    /**
     * @brief 現在のフレームレート（FPS）を取得します。
     * @return f32 計測された FPS 値
     */
    [[nodiscard]] auto fps() const noexcept -> f32 { return fps_counter_.fps(); }

  private:
    explicit clock() noexcept = default;

    detail::fps_counter fps_counter_;
    f64                 last_time_{};
    f64                 delta_time_{};
};

/**
 * @brief tick インスタンスを生成するフリーのファクトリ関数
 * @return tick 生成された tick インスタンス
 */
[[nodiscard]] inline auto make_tick() noexcept -> clock { return clock::make(); }
}  // namespace rin