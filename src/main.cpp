#include <cassert>
#include <glm/trigonometric.hpp>

#include "glad/glad.h"

#include "shape.hpp"
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

        auto quad = Quad{};

        quad.size = {1.f, 1.f};

        quad.color = {2.f, 3.f, 4.f};

        while (window.is_open()) {
            window.begin_frame(0.3f, 0.2f, 0.1f, 1.f);

            window.draw(quad);

            window.display();
        }
    }
};

auto main() -> int {
    auto engine = Engine{};
    engine.run();
}