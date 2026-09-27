#pragma once

#include <chrono>
#include <thread>

#include "glad/glad.h"

#include "others/type.hpp"

namespace rin {
class timer final {
    using clock = std::chrono::high_resolution_clock;

  public:
    auto tick() noexcept -> f32 {
        const auto now = clock::now();
        if (target_frame_duration_ > std::chrono::duration<f32>::zero()) {
            const auto elapsed        = now - last_time_;
            const auto sleep_duration = target_frame_duration_ - elapsed;
            std::this_thread::sleep_for(sleep_duration);
        }

        const auto current_time = clock::now();
        const auto delta        = current_time - last_time_;
        last_time_              = current_time;
        delta_time_             = static_cast<f32>(delta.count());
        return delta_time_;
    }

    void target_fps(const f32 target) noexcept {
        if (target > 0.f)
            target_frame_duration_ = std::chrono::duration<f32>(1.f / target);
        else
            target_frame_duration_ = std::chrono::duration<f32>::zero();
    }

    [[nodiscard]] auto target_fps() const noexcept -> f32 { return target_frame_duration_.count(); }

    [[nodiscard]] constexpr auto delta_time() const noexcept -> f32 {
        return static_cast<f32>(delta_time_);
    }

  private:
    std::chrono::duration<f32> target_frame_duration_{std::chrono::duration<f32>::zero()};
    clock::time_point          last_time_;
    f32                        delta_time_{};
};
}  // namespace rin