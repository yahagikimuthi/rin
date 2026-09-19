#include <cassert>
#include <utility>

#include "glad/glad.h"

#include "mesh.hpp"
#include "shader.hpp"
#include "window.hpp"

// 1. バーテックスシェーダー（頂点の位置をそのまま通過させる）
constexpr const char* vertex_shader_source = R"(
    #version 450 core
    layout (location = 0) in vec3 aPos;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
)";

// 2. フラグメントシェーダー（ピクセルをオレンジ色に塗る）
constexpr const char* fragment_shader_source = R"(
    #version 450 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)";

using namespace rin;
auto main() -> int {
    auto windowResult = Window::create(800, 600, "Title");
    if (not windowResult) return 1;
    auto window = std::move(*windowResult);

    auto shader_result = Shader::createFromSource(vertex_shader_source, fragment_shader_source);
    if (not shader_result) return 1;
    auto shader = std::move(*shader_result);

    constexpr auto vertices = std::array<float, 9>{
        -0.5f,  // 左下 X
        -0.5f,  //     Y
        0.0f,   //     Z

        0.5f,  // 右下
        -0.5f,
        0.0f,

        0.0f,  // 上
        0.5f,
        0.0f
    };

    auto triangle = Mesh{vertices, 3};
    while (window.is_open()) {
        window.clear();

        shader.use();
        triangle.draw();

        window.display();
    }
}