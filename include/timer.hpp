#pragma once

#include "glad/glad.h"

#include "GLFW/glfw3.h"
#include "type.hpp"

namespace rin {
class Timer final {
  public:
    static auto tick() noexcept -> f32 {
        const auto current_time = glfwGetTime();
        delta_time_             = current_time - last_time_;
        last_time_              = current_time;

        return static_cast<f32>(delta_time_);
    }

  private:
    static inline constinit f64 last_time_{};
    static inline constinit f64 delta_time_{};
};
}  // namespace rin