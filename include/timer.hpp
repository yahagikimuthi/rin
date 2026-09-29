#pragma once

#include "detail/graphics.hpp"
#include "type.hpp"

namespace rin {
class timer final {
  public:
    [[nodiscard]] static auto make() noexcept -> timer { return timer{}; }

    auto tick() noexcept -> f32 {
        const auto current_time = glfwGetTime();
        delta_time_             = current_time - last_time_;
        last_time_              = current_time;
        return static_cast<f32>(delta_time_);
    }

    [[nodiscard]] constexpr auto delta_time() const noexcept -> f32 {
        return static_cast<f32>(delta_time_);
    }

  private:
    explicit timer() noexcept = default;

    f64 last_time_{};
    f64 delta_time_{};
};

[[nodiscard]] auto make_timer() noexcept -> timer { return timer::make(); }
}  // namespace rin