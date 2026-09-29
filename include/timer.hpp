#pragma once

#include "detail/fps_counter.hpp"
#include "detail/graphics.hpp"
#include "types.hpp"

namespace rin {
class timer final {
  public:
    [[nodiscard]] static auto make() noexcept -> timer { return timer{}; }

    auto tick() noexcept -> f32 {
        const auto current_time = glfwGetTime();
        delta_time_             = current_time - last_time_;
        last_time_              = current_time;
        fps_counter_.update();
        return static_cast<f32>(delta_time_);
    }

    [[nodiscard]] constexpr auto delta_time() const noexcept -> f32 {
        return static_cast<f32>(delta_time_);
    }

    [[nodiscard]] auto fps() const noexcept -> f32 { return fps_counter_.fps(); }

  private:
    explicit timer() noexcept = default;

    detail::fps_counter fps_counter_;
    f64                 last_time_{};
    f64                 delta_time_{};
};

[[nodiscard]] auto make_timer() noexcept -> timer { return timer::make(); }
}  // namespace rin