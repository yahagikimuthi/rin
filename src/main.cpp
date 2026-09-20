#include <cassert>
#include <utility>

#include "glad/glad.h"

#include "shape.hpp"
#include "window.hpp"

using namespace rin;

auto main() -> int {
    auto windowResult = Window::create(800, 600, "Title");
    if (not windowResult) return 1;
    auto window = std::move(*windowResult);

    auto quad = Quad{1.f, 5.f};

    while (window.is_open()) {
        window.begin_frame();

        window.draw(quad);

        window.display();
    }
}