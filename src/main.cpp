#include <cassert>
#include <utility>

#include "glad/glad.h"

#include "mesh.hpp"
#include "shader.hpp"
#include "type.hpp"
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

    // 1. 四角形の4頂点 (X, Y, Z)
    constexpr auto vertices = std::array<f32, 12>{
        -0.5f,  // 0: 左下
        -0.5f,
        0.0f,
        0.5f,  // 1: 右下
        -0.5f,
        0.0f,
        0.5f,  // 2: 右上
        0.5f,
        0.0f,
        -0.5f,  // 3: 左上
        0.5f,
        0.0f
    };

    // 2. 頂点をつなぐインデックス（時計回りまたは反時計回りで統一）
    constexpr auto indices = std::array<u32, 6>{
        0,  // 1つ目の三角形 (左下 -> 右下 -> 右上)
        1,
        2,
        2,  // 2つ目の三角形 (右上 -> 左上 -> 左下)
        3,
        2
    };

    auto triangle = Mesh{vertices, indices, 3};
    while (window.is_open()) {
        window.clear();

        shader.use();
        triangle.draw();

        window.display();
    }
}