#include "rin/color.hpp"
#include "rin/vertex_vector.hpp"
#include "rin/window.hpp"

/**
 * @brief 三角形を描画する最も基本的なプログラム
 */
auto main() -> int {
    // ウィンドウの生成, 失敗した場合異常終了
    auto window_res = rin::try_make_window(800, 600);
    if (not window_res) window_res.error().panic();

    auto& window = *window_res;

    // 三角形の頂点配列を作成
    auto vertices = rin::vertex_vector{rin::primitive_triangles};
    vertices.emplace_back(rin::vec2{.x = 300, .y = 300}, rin::red);
    vertices.emplace_back(rin::vec2{.x = 500, .y = 300}, rin::blue);
    vertices.emplace_back(rin::vec2{.x = 400, .y = 400}, rin::green);

    while (window.is_open()) {
        window.poll_events();
        window.begin_render();
        window.draw(vertices);
        window.end_render();
    }
}