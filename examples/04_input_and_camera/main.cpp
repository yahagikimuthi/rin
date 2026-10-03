#include "rin/clock.hpp"
#include "rin/key.hpp"
#include "rin/mouse.hpp"
#include "rin/vertex_vector.hpp"
#include "rin/window.hpp"

auto main() -> int {
    auto win_res = rin::try_make_window(800, 600);
    if (not win_res) win_res.error().panic();
    auto& win = *win_res;

    auto vertices = rin::vertex_vector{rin::primitive_triangles};
    vertices.emplace_back(rin::vec2{.x = 300, .y = 300}, rin::red);
    vertices.emplace_back(rin::vec2{.x = 500, .y = 300}, rin::blue);
    vertices.emplace_back(rin::vec2{.x = 400, .y = 400}, rin::green);

    auto timer = rin::make_clock();

    while (win.is_open()) {
        win.poll_events();

        timer.tick();
        const auto delta_time = timer.delta_time();
        const auto move       = delta_time * 200.f;
        const auto camera_pos = win.camera_position();

        if (win.is_key_down(rin::key_a)) {
            win.camera_position(camera_pos.x - move, camera_pos.y);
        } else if (win.is_key_down(rin::key_s)) {
            win.camera_position(camera_pos.x, camera_pos.y - move);
        } else if (win.is_key_down(rin::key_d)) {
            win.camera_position(camera_pos.x + move, camera_pos.y);
        } else if (win.is_key_down(rin::key_w)) {
            win.camera_position(camera_pos.x, camera_pos.y + move);
        }
        if (win.is_mouse_scroll(rin::scroll_up)) {
            win.zoom_camera(1.1f);
        } else if (win.is_mouse_scroll(rin::scroll_down)) {
            win.zoom_camera(0.9f);
        }

        win.begin_render();
        win.draw(vertices);
        win.end_render();
    }
}