#pragma once

#include <array>
#include <ranges>

#include "rin/audio.hpp"

namespace gm {
class SoundManager final {
  public:
    explicit SoundManager() {
        engine_.master_volume(1.f);
        for (auto& sound : clash_sounds_) {
            sound.volume(1.f);
        }
    }

    SoundManager(const SoundManager&) noexcept                    = delete;
    auto operator=(const SoundManager&) noexcept -> SoundManager& = delete;
    SoundManager(SoundManager&&) noexcept                         = delete;
    auto operator=(SoundManager&&) noexcept -> SoundManager&      = delete;
    ~SoundManager() noexcept                                      = default;

    void crash() noexcept {
        auto valid_sounds =
            clash_sounds_ | std::views::filter([](const rin::sound& sound) noexcept -> bool {
                return not sound.is_playing();
            });
        if (valid_sounds.empty()) return;

        valid_sounds.front().play();
    }

  private:
    rin::audio_engine         engine_{rin::try_make_audio_engine().value()};
    std::array<rin::sound, 8> clash_sounds_{
        engine_.try_load_sound("crash.mp3").value(),
        engine_.try_load_sound("crash.mp3").value(),
        engine_.try_load_sound("crash.mp3").value(),
        engine_.try_load_sound("crash.mp3").value(),
        engine_.try_load_sound("crash.mp3").value(),
        engine_.try_load_sound("crash.mp3").value(),
        engine_.try_load_sound("crash.mp3").value(),
        engine_.try_load_sound("crash.mp3").value()
    };
};
}  // namespace gm