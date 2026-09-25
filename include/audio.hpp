#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <utility>

#define MINIAUDIO_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "miniaudio.h"
#pragma GCC diagnostic pop

#include "others/error.hpp"
#include "others/type.hpp"

namespace rin {
class sound final {
    friend class audio_engine;

  public:
    sound(const sound&) noexcept                    = delete;
    auto operator=(const sound&) noexcept -> sound& = delete;

    sound(sound&& other) noexcept : sound_{std::move(other.sound_)} {}
    auto operator=(sound&& other) noexcept -> sound& {
        if (this == &other) return *this;

        destroy();

        sound_ = std::move(other.sound_);
        return *this;
    }
    ~sound() noexcept = default;

    [[nodiscard]] auto is_playing() const noexcept -> bool {
        return sound_ and ma_sound_is_playing(sound_.get()) == MA_TRUE;
    }

    void play() noexcept {
        if (sound_) ma_sound_start(sound_.get());
    }
    void stop() noexcept {
        if (sound_) ma_sound_stop(sound_.get());
    }
    void looping(const bool loop) noexcept {
        if (sound_) ma_sound_set_looping(sound_.get(), loop ? MA_TRUE : MA_FALSE);
    }
    void volume(const f32 volume) noexcept {
        if (sound_) ma_sound_set_volume(sound_.get(), volume);
    }

  private:
    explicit sound() noexcept = default;
    void destroy() noexcept {
        if (not sound_) return;
        ma_sound_uninit(sound_.get());
        sound_ = nullptr;
    }

    std::unique_ptr<ma_sound> sound_{std::make_unique<ma_sound>()};
};

class audio_engine final {
  public:
    [[nodiscard]] static auto create() noexcept -> std::expected<audio_engine, error> {
        auto       engine = std::make_unique<ma_engine>();
        const auto result = ma_engine_init(nullptr, engine.get());
        if (result != MA_SUCCESS)
            return make_error(runtime_error, "Failed to initialize miniaudio engine.");

        return audio_engine{std::move(engine)};
    }

    audio_engine(const audio_engine&) noexcept                    = delete;
    auto operator=(const audio_engine&) noexcept -> audio_engine& = delete;

    audio_engine(audio_engine&& other) noexcept : engine_{std::move(other.engine_)} {}
    auto operator=(audio_engine&& other) noexcept -> audio_engine& {
        if (this == &other) return *this;

        destroy();

        engine_ = std::move(other.engine_);
        return *this;
    }

    ~audio_engine() noexcept { destroy(); }

    [[nodiscard]] auto try_load_sound(const std::filesystem::path& path) noexcept
        -> std::expected<sound, error> {
        if (not engine_) return make_error(logic_error, "Audio engine is not initialized.");

        auto       sound_obj = sound{};
        const auto result    = ma_sound_init_from_file(
            engine_.get(), path.string().c_str(), 0, nullptr, nullptr, sound_obj.sound_.get()
        );
        if (result != MA_SUCCESS) return make_error(logic_error, "Failed to load sound file.");

        return sound_obj;
    }

    void master_volume(const f32 volume) noexcept {
        if (engine_) ma_engine_set_volume(engine_.get(), volume);
    }

  private:
    explicit audio_engine(std::unique_ptr<ma_engine> engine) noexcept
        : engine_{std::move(engine)} {}

    void destroy() noexcept {
        if (not engine_) {
            return;
        }
        ma_engine_uninit(engine_.get());
        engine_.reset();
    }

    std::unique_ptr<ma_engine> engine_;
};

[[nodiscard]] inline auto try_make_audio_engine() noexcept -> std::expected<audio_engine, error> {
    return audio_engine::create();
}
}  // namespace rin