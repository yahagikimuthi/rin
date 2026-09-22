#include <cassert>
#include <glm/trigonometric.hpp>

#include "glad/glad.h"

#include "shape.hpp"
#include "timer.hpp"
#include "window.hpp"

using namespace rin;

class Engine {
  public:
    static void run() noexcept {
        auto window_res = Window::create(800, 600, "Title");
        if (not window_res) {
            window_res.error().what();
            return;
        }
        auto& window = *window_res;

        auto quad  = Quad{};
        auto quad2 = Quad{};
        auto shape = Shape{8};

        quad2.position(100.f, 100.f);

        quad.size(100.f, 200.f);

        quad.color(0.2f, 0.4f, 0.1f, 1.f);
        quad.rotation_radian() = 3.1f;

        constexpr auto speed = 100.f;

        while (window.is_open()) {
            const auto delta_time = Timer::tick();

            window.poll_events();

            if (Input::is_key_down(Key::w)) {
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
    auto engine = Engine{};
    engine.run();
}