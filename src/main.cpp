#include <cassert>
#include <utility>

#include "glad/glad.h"

#include "shape.hpp"
#include "window.hpp"

using namespace rin;

auto main() -> int {
    auto windowResult = Window::create(800, 600, "Title");
    if (not windowResult) {
        windowResult.error().what();
        return 1;
    }
    auto window = std::move(*windowResult);

    auto quadResult = QuadMesh::create(1.f, 5.f);
    if (not quadResult) {
        quadResult.error().what();
        return 1;
    }
    auto quad = std::move(*quadResult);

    while (window.is_open()) {
        window.begin_frame();

        window.draw(quad);

        window.display();
    }
}