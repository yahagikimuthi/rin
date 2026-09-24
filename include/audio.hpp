#pragma once

#include <expected>
#include <filesystem>
#include <utility>
#include "others/type.hpp"

#define MINIAUDIO_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "miniaudio.h"
#pragma GCC diagnostic pop

#include "others/error.hpp"

namespace rin {
class sound final {
    friend class audio_engine;

  public:
    sound(const sound&) noexcept                    = delete;
    auto operator=(const sound&) noexcept -> sound& = delete;

    sound(sound&& other) noexcept
        : sound_{std::exchange(other.sound_, ma_sound{})},
          is_initialized_{std::exchange(other.is_initialized_, false)} {}
    auto operator=(sound&& other) noexcept -> sound& {
        if (this == &other) return *this;

        destroy();

        sound_          = std::exchange(other.sound_, ma_sound{});
        is_initialized_ = std::exchange(other.is_initialized_, false);
        return *this;
    }
    ~sound() noexcept = default;

    [[nodiscard]] auto is_playing() const noexcept -> bool {
        return is_initialized_ and ma_sound_is_playing(&sound_) == MA_TRUE;
    }

    void play() noexcept {
        if (is_initialized_) ma_sound_start(&sound_);
    }
    void stop() noexcept {
        if (is_initialized_) ma_sound_stop(&sound_);
    }
    void looping(const bool loop) noexcept {
        if (is_initialized_) ma_sound_set_looping(&sound_, loop ? MA_TRUE : MA_FALSE);
    }
    void volume(const f32 volume) noexcept {
        if (is_initialized_) ma_sound_set_volume(&sound_, volume);
    }

  private:
    explicit sound() noexcept = default;
    void destroy() noexcept {
        if (not is_initialized_) return;
        ma_sound_uninit(&sound_);
        is_initialized_ = false;
    }

    ma_sound sound_{};
    bool     is_initialized_{false};
};

class audio_engine final {
  public:
    [[nodiscard]] static auto create() noexcept -> std::expected<audio_engine, error> {
        auto       engine = ma_engine{};
        const auto result = ma_engine_init(nullptr, &engine);
        if (result != MA_SUCCESS)
            return error::create(runtime_error, "Failed to initialize miniaudio engine.");

        return audio_engine{engine};
    }

    audio_engine(const audio_engine&) noexcept                    = delete;
    auto operator=(const audio_engine&) noexcept -> audio_engine& = delete;

    audio_engine(audio_engine&& other) noexcept
        : engine_{std::exchange(other.engine_, ma_engine{})},
          is_initialized_{std::exchange(other.is_initialized_, false)} {}
    auto operator=(audio_engine&& other) noexcept -> audio_engine& {
        if (this == &other) return *this;

        destroy();

        engine_         = std::exchange(other.engine_, ma_engine{});
        is_initialized_ = std::exchange(other.is_initialized_, false);
        return *this;
    }

    ~audio_engine() noexcept { destroy(); }

    [[nodiscard]] auto load_sound(const std::filesystem::path& path) noexcept
        -> std::expected<sound, error> {
        if (not is_initialized_)
            return error::create(logic_error, "Audio engine is not initialized.");

        auto       sound_obj = sound{};
        const auto result    = ma_sound_init_from_file(
            &engine_, path.string().c_str(), 0, nullptr, nullptr, &sound_obj.sound_
        );
        if (result != MA_SUCCESS) return error::create(logic_error, "Failed to load sound file.");

        sound_obj.is_initialized_ = true;
        return sound_obj;
    }

    void master_volume(const f32 volume) noexcept {
        if (is_initialized_) ma_engine_set_volume(&engine_, volume);
    }

  private:
    explicit audio_engine(const ma_engine engine) noexcept
        : engine_{engine}, is_initialized_{true} {}

    void destroy() noexcept {
        if (not is_initialized_) return;
        ma_engine_uninit(&engine_);
        is_initialized_ = false;
    }

    ma_engine engine_{};
    bool      is_initialized_{false};
};
}  // namespace rin