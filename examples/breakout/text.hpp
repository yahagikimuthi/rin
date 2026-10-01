#pragma once

#include <cassert>
#include <string>

#include "rin/color.hpp"
#include "rin/font.hpp"
#include "rin/window.hpp"

#include "../examples/breakout/setting.hpp"

namespace gm {

class TextManager {
  public:
    explicit TextManager() {
        {
            state_.scale(2.f, 2.f);
            state_.position(window_size.width / 2, window_size.height / 1.75f);
        }
        {
            score_.scale(0.8f, 0.8f);
            score_.position(window_size.width / 2, window_size.height / 2.5f);
            score_.color(rin::white);
        }
        {
            explanation_.scale(0.8f, 0.8f);
            explanation_.position(window_size.width / 2, window_size.height / 3);
            explanation_.color(rin::white);
            explanation_.string("You can restart to press escape button.");
            const auto extent = explanation_.calc_extent();
            explanation_.origin(extent.width / 2, extent.height / 2);
        }
        {
            wait_.string("Click to start!");
            wait_.position(window_size.width / 2, window_size.height / 2.5f);
            wait_.color(rin::white);
            const auto extent = wait_.calc_extent();
            wait_.origin(extent.width / 2, extent.height / 2);
        }
    }

    TextManager(const TextManager&)                             = delete;
    auto operator=(const TextManager&) noexcept -> TextManager& = delete;
    TextManager(TextManager&&) noexcept                         = delete;
    auto operator=(TextManager&&) noexcept -> TextManager&      = delete;
    ~TextManager() noexcept                                     = default;

    void wait() noexcept { is_waiting_ = true; }

    void start() noexcept { is_waiting_ = false; }

    void game_clear(const u32 point) noexcept {
        state_.string("Game Clear");
        state_.color(rin::green);
        const auto extent = state_.calc_extent();
        state_.origin(extent.width / 2, extent.height / 2);

        score(point);
    }

    void game_over(const u32 point) noexcept {
        state_.string("Game Over");
        state_.color(rin::red);
        const auto extent = state_.calc_extent();
        state_.origin(extent.width / 2, extent.height / 2);

        score(point);
    }

    void reset() noexcept {
        state_.string("");
        score_.string("");
        state_.color(rin::black);
    }

    void draw(rin::window& win) noexcept {
        if (is_waiting_) {
            win.draw(wait_);
            return;
        }
        if (score_.string().empty()) return;
        win.draw(state_);
        win.draw(score_);
        win.draw(explanation_);
    }

  private:
    void score(const u32 num) noexcept {
        score_.string("Destroyed Blocks: " + std::to_string(num));
        const auto extent = score_.calc_extent();
        score_.origin(extent.width / 2, extent.height / 2);
    }

    rin::font font_{rin::try_make_font("../examples/breakout/DejaVuSans.ttf").value()};
    rin::text wait_{rin::make_text(font_)};
    rin::text state_{rin::make_text(font_)};
    rin::text score_{rin::make_text(font_)};
    rin::text explanation_{rin::make_text(font_)};
    bool      is_waiting_{true};
};
}  // namespace gm