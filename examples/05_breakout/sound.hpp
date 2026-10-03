#pragma once

#include <array>
#include <ranges>
#include <utility>

#include "rin/audio.hpp"
#include "rin/error.hpp"

namespace gm {
class SoundManager final {
  public:
    [[nodiscard]] static auto make() noexcept -> SoundManager {
        auto engine = rin::try_make_audio_engine();
        if (not engine) engine.error().panic();

        auto sounds_res = std::array<std::expected<rin::sound, rin::error>, 8>{
            engine->try_load_sound("../examples/05_breakout/crash.mp3"),
            engine->try_load_sound("../examples/05_breakout/crash.mp3"),
            engine->try_load_sound("../examples/05_breakout/crash.mp3"),
            engine->try_load_sound("../examples/05_breakout/crash.mp3"),
            engine->try_load_sound("../examples/05_breakout/crash.mp3"),
            engine->try_load_sound("../examples/05_breakout/crash.mp3"),
            engine->try_load_sound("../examples/05_breakout/crash.mp3"),
            engine->try_load_sound("../examples/05_breakout/crash.mp3")
        };

        for (auto& sound : sounds_res) {
            if (not sound) sound.error().panic();
        }

        return SoundManager{
            std::move(*engine),
            std::array<rin::sound, 8>{
                std::move(*sounds_res[0]),
                std::move(*sounds_res[0]),
                std::move(*sounds_res[0]),
                std::move(*sounds_res[0]),
                std::move(*sounds_res[0]),
                std::move(*sounds_res[0]),
                std::move(*sounds_res[0]),
                std::move(*sounds_res[0])
            }
        };
    };

    void crash() noexcept {
        auto valid_sounds =
            clash_sounds_ | std::views::filter([](const rin::sound& sound) noexcept -> bool {
                return not sound.is_playing();
            });
        if (valid_sounds.empty()) return;

        valid_sounds.front().play();
    }

  private:
    explicit SoundManager(rin::audio_engine engine, std::array<rin::sound, 8> sounds) noexcept
        : engine_{std::move(engine)}, clash_sounds_(std::move(sounds)) {}

    rin::audio_engine         engine_;
    std::array<rin::sound, 8> clash_sounds_;
};
}  // namespace gm