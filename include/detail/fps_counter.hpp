#pragma once

#include <chrono>

#include "types.hpp"

namespace rin::detail {

class fps_counter final {
  public:
    using clock      = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using duration   = std::chrono::duration<f32>;

    explicit fps_counter() noexcept = default;

    void update() noexcept {
        const auto now = clock::now();

        delta_time_ = std::chrono::duration_cast<duration>(now - last_time_).count();
        last_time_  = now;

        frame_count_++;

        const auto time_since_fps_update =
            std::chrono::duration_cast<duration>(now - fps_last_updated_).count();
        if (time_since_fps_update >= update_interval_) {
            current_fps_      = static_cast<f32>(frame_count_) / time_since_fps_update;
            current_frame_ms_ = (time_since_fps_update / static_cast<f32>(frame_count_)) * 1000.0f;

            frame_count_      = 0;
            fps_last_updated_ = now;
            has_updated_      = true;
        } else {
            has_updated_ = false;
        }
    }

    [[nodiscard]] auto fps() const noexcept -> f32 { return current_fps_; }

  private:
    f32 update_interval_{1.0f};  // 表示更新の間隔 (秒)

    time_point last_time_{clock::now()};
    time_point fps_last_updated_{last_time_};

    f32 delta_time_{0.0f};
    f32 current_fps_{0.0f};
    f32 current_frame_ms_{0.0f};

    std::uint32_t frame_count_{0};
    bool          has_updated_{false};
};

}  // namespace rin::detail