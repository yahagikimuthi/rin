#include <glm/ext/matrix_float2x2.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/trigonometric.hpp>

#include "rin/clock.hpp"
#include "rin/collision.hpp"
#include "rin/color.hpp"
#include "rin/window.hpp"

#include "examples/breakout/ball.hpp"
#include "examples/breakout/block.hpp"
#include "examples/breakout/paddle.hpp"
#include "examples/breakout/setting.hpp"
#include "examples/breakout/sound.hpp"
#include "examples/breakout/text.hpp"

namespace gm {
class Engine final {
  public:
    explicit Engine(rin::window win) : window_{std::move(win)} {}
    void run() noexcept {
        while (window_.is_open()) {
            window_.poll_events();
            main_loop();
            display();
        }
    }

  private:
    void main_loop() noexcept;

    [[nodiscard]] auto is_ball_collision_window_and_process() noexcept -> bool {
        const auto ball_pos = ball_.position();
        const auto ball_v   = ball_.velocity();
        if (ball_pos.y + ball_radius >= window_size.height) {
            ball_.velocity(ball_v.x, -ball_v.y);
            ball_.position(ball_pos.x, window_size.height - ball_radius);
            return true;
        }
        if (ball_pos.y + ball_radius < 0.f) {
            return true;
        }
        if (ball_pos.x - ball_radius <= 0.f) {
            ball_.velocity(-ball_v.x, ball_v.y);
            ball_.position(0.f + ball_radius, ball_pos.y);
            return true;
        }
        if (ball_pos.x + ball_radius >= window_size.width) {
            ball_.velocity(-ball_v.x, ball_v.y);
            ball_.position(window_size.width - ball_radius, ball_pos.y);
            return true;
        }
        return false;
    }

    [[nodiscard]] auto is_ball_collision_paddle_and_process() noexcept -> bool {
        const auto ball_pos = ball_.position();
        if (ball_pos.y - ball_radius > (paddle_center.y + (paddle_size.height / 2))) return false;

        const auto paddle_pos   = paddle_.position();
        const auto ball_v       = ball_.velocity();
        const auto paddle_bound = rin::make_aabb_bound(paddle_pos, paddle_size);
        const auto ball_bound   = rin::make_circle_bound(ball_pos, ball_radius);
        const auto hit          = rin::intersects(paddle_bound, ball_bound);
        if (not hit) return false;
        ball_.position(ball_pos + hit->resolution_vector * hit->penetration);

        if (hit->resolution_vector.y < 0.f) {  // 下方向への反発(ほとんどゲームオーバー)
            const auto dot =
                (ball_v.x * hit->resolution_vector.x) + (ball_v.y * hit->resolution_vector.y);
            ball_.velocity(ball_v - (hit->resolution_vector * 2.f * dot));
            return true;
        }

        // パドルとの相対位置を基に回転を加える
        const auto hit_x_from_paddle = ball_pos.x - paddle_pos.x;
        const auto hit_x_ratio       = hit_x_from_paddle / (paddle_size.width / 2);
        const auto rolling           = -glm::radians(45.f * hit_x_ratio);
        const auto add_by_hit =
            glm::rotate(glm::vec2{0.f, ball_v.abs()}, rolling);  // 鉛直上向きの速度をrolling分回転
        ball_.velocity(add_by_hit.x, add_by_hit.y);
        return true;
    }

    [[nodiscard]] auto is_ball_collision_blocks_and_process() noexcept -> bool {
        const auto ball_pos = ball_.position();
        if (ball_pos.y + ball_radius <
            blocks_.vec[0, block_col - 1].position.y - (block_size.height / 2))
            return false;

        const auto ball_v     = ball_.velocity();
        const auto ball_bound = rin::make_circle_bound(ball_pos, ball_radius);

        for (const auto row_idx : std::views::indices(static_cast<std::size_t>(block_row))) {
            for (const auto col_idx : std::views::indices(static_cast<std::size_t>(block_col))) {
                if (not blocks_.vec[row_idx, col_idx].is_valid) continue;

                const auto block_pos   = blocks_.vec[row_idx, col_idx].position;
                const auto block_bound = rin::make_aabb_bound(block_pos, block_size);

                const auto hit = rin::intersects(block_bound, ball_bound);
                if (not hit) continue;

                blocks_.vec[row_idx, col_idx].is_valid = false;
                const auto dot =
                    (ball_v.x * hit->resolution_vector.x) + (ball_v.y * hit->resolution_vector.y);
                ball_.velocity(ball_v - (hit->resolution_vector * 2.f * dot));
                score_ += 10;
                sound_.crash();

                break;
            }
        }

        return true;
    }

    void display() noexcept {
        window_.begin_render(rin::gray);
        paddle_.draw(window_);
        ball_.draw(window_);
        blocks_.draw(window_);
        text_.draw(window_);
        window_.end_render();
    }

    rin::window  window_;
    SoundManager sound_;
    rin::clock   clock_{rin::make_clock()};
    Paddle       paddle_;
    Ball         ball_;
    Blocks       blocks_;
    TextManager  text_;
    u32          score_{};
    GameState    state_{GameState::waiting};
};

void Engine::main_loop() noexcept {
    clock_.tick();
    const auto delta_time = clock_.delta_time();
    const auto mouse      = window_.mouse_position();

    if (state_ == GameState::clear or state_ == GameState::over) {
        if (not window_.is_key_pressed(rin::key_escape)) return;
        state_ = GameState::waiting;
        text_.wait();
        ball_.velocity(0.f, 0.f);
        text_.reset();
        blocks_.reset();
    }

    paddle_.x(mouse.x);

    if (state_ == GameState::waiting) {
        if (window_.is_mouse_pressed(rin::mouse_left)) {
            state_ = GameState::playing;
            ball_.velocity(0.f, init_ball_speed);
            text_.start();
        }
        ball_.position(mouse.x, paddle_.top().y + ball_radius);
        return;
    }

    ball_.move(delta_time);

    for (const auto _ : std::views::indices(3)) {
        const auto is_ball_collision_window = is_ball_collision_window_and_process();
        const auto is_ball_collision_paddle = is_ball_collision_paddle_and_process();
        const auto is_ball_collision_blocks = is_ball_collision_blocks_and_process();

        if (not is_ball_collision_window and not is_ball_collision_paddle and
            not is_ball_collision_blocks)
            break;
    }

    if (not blocks_.is_any_blocks_valid()) {
        text_.game_clear(block_row * block_col);
        state_ = GameState::clear;
        return;
    }

    const auto ball_pos = ball_.position();
    if (ball_pos.y + ball_radius < 0.f) {
        text_.game_over(blocks_.calc_invalid_blocks());
        state_ = GameState::over;
    }
}
}  // namespace gm

auto main() -> int {
    auto win = rin::try_make_window(800, 600, "Breakout");
    if (not win) win.error().panic();

    auto engine = gm::Engine{std::move(*win)};
    engine.run();
}