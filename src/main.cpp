#include <cassert>
#include <glm/trigonometric.hpp>

#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "stb_image.h"
#pragma GCC diagnostic pop

#include "shape.hpp"
#include "timer.hpp"
#include "window.hpp"

class Engine {
  public:
    static void run() noexcept {
        auto window_res = rin::window::create(800, 600, "Title");
        if (not window_res) {
            window_res.error().what();
            return;
        }
        auto& window = *window_res;

        auto quad  = rin::polygon{4};
        auto quad2 = rin::polygon{4};
        auto shape = rin::polygon{5};

        quad2.position(100.f, 100.f);

        quad.size(100.f, 200.f);

        quad.color(rin::color::red);
        quad.rotation_radian() = 3.1f;

        constexpr auto speed = 100.f;

        while (window.is_open()) {
            const auto delta_time = rin::timer::delta_time();

            window.poll_events();

            if (rin::input::is_key_down(rin::key::w)) {
                quad.position().y += speed * delta_time;
            }
            window.camera_position(quad.position());

            window.clear(0.3f, 0.2f, 0.1f, 1.f);
            window.draw(quad);
            window.draw(shape);
            window.draw(quad2);
            window.display();
        }
    }
};

auto main() -> int {
    auto window_res = rin::window::create(800, 600);
    if (not window_res) return 1;
    auto& window = *window_res;

    while (window.is_open()) {
        window.poll_events();
    }
}