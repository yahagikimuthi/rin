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
#include "rin/detail/miniaudio.h"
#pragma GCC diagnostic pop

#include "rin/error.hpp"
#include "rin/types.hpp"

/**
 * @file audio.hpp
 * @brief miniaudioをバックエンドとするsound及びaudio_engineクラス
 */

namespace rin {
/**
 * @class sound
 * @brief 個別のサウンド再生および設定（再生、停止、ループ、音量調整など）を行うクラス
 *
 * @details `audio_engine::try_load_sound` を通じて生成されます。
 * 内部で miniaudio の `ma_sound` を保持しており、デストラクタ呼び出し時に `ma_sound_uninit`
 * が自動的に実行されます。
 * リソースの重複解放を防ぐため、コピーは不可でムーブセマンティクスのみサポートします。
 */
class sound final {
    /// `audio_engine` からの非公開コンストラクタ呼び出しおよび内部 `sound_`
    /// メンバーへのアクセスを許可
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

    /**
     * @brief デストラクタ
     * @details 所有している `ma_sound` の終了処理（`ma_sound_uninit`）を実行します。
     */
    ~sound() noexcept { destroy(); }

    /**
     * @brief 現在サウンドが再生中であるかを確認します。
     *
     * @return true 再生中の場合
     * @return false 停止中またはインスタンスが無効な場合
     */
    [[nodiscard]] auto is_playing() const noexcept -> bool {
        return sound_ and ma_sound_is_playing(sound_.get()) == MA_TRUE;
    }

    /**
     * @brief サウンドの再生を開始（または一時停止状態から再開）します。
     */
    void play() noexcept {
        if (sound_) ma_sound_start(sound_.get());
    }

    /**
     * @brief サウンドの再生を停止します。
     */
    void stop() noexcept {
        if (sound_) ma_sound_stop(sound_.get());
    }

    /**
     * @brief ループ再生の有効／無効を設定します。
     *
     * @param loop true の場合はループ再生を有効化、false の場合は1回再生で終了
     */
    void looping(const bool loop) noexcept {
        if (sound_) ma_sound_set_looping(sound_.get(), loop ? MA_TRUE : MA_FALSE);
    }

    /**
     * @brief 個別の音量を設定します。
     *
     * @param volume 音量（0.0f で消音、1.0f で標準）
     */
    void volume(const f32 volume) noexcept {
        if (sound_) ma_sound_set_volume(sound_.get(), volume);
    }

  private:
    explicit sound() noexcept = default;

    void destroy() noexcept {
        if (not sound_) {
            return;
        }
        ma_sound_uninit(sound_.get());
        sound_.reset();
    }

    std::unique_ptr<ma_sound> sound_{std::make_unique<ma_sound>()};
};

/**
 * @class audio_engine
 * @brief オーディオの再生・管理を行うメインエンジンクラス
 *
 * @details miniaudioの `ma_engine` を所有し、サウンドのロードやマスターボリュームの設定を行います。
 * RAIIに基づき、破棄時には自動的にオーディオエンジンの終了処理が呼び出されます。
 * コピーは禁止されており、ムーブのみ可能です。
 */
class audio_engine final {
  public:
    /**
     * @brief オーディオエンジンインスタンスの作成を試みます。
     *
     * @return std::expected<audio_engine, error>
     *         成功した場合は `audio_engine` のインスタンス、失敗した場合はエラー情報を返します。
     */
    [[nodiscard]] static auto try_make() noexcept -> std::expected<audio_engine, error> {
        auto       engine = std::make_unique<ma_engine>();
        const auto result = ma_engine_init(nullptr, engine.get());
        if (result != MA_SUCCESS) {
            return make_error(runtime_error, "Failed to initialize miniaudio engine.");
        }

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

    /**
     * @brief デストラクタ
     * @details 所有している `ma_engine` の解放処理（`ma_engine_uninit`）を実行します。
     */
    ~audio_engine() noexcept { destroy(); }

    /**
     * @brief ファイルパスを指定してサウンドリソースをロードします。
     *
     * @param path ロードする音声ファイルのパス
     * @return std::expected<sound, error>
     *         成功した場合は `sound` オブジェクト、失敗した場合はエラー情報を返します。
     */
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

    /**
     * @brief 全体のマスターボリュームを設定します。
     *
     * @param volume 音量（0.0f で消音、1.0f で標準）
     */
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

/**
 * @brief `audio_engine` の生成を試みるヘルパー関数
 *
 * @return std::expected<audio_engine, error>
 *         成功した場合は `audio_engine` のインスタンス、失敗した場合はエラー情報を返します。
 */
[[nodiscard]] inline auto try_make_audio_engine() noexcept -> std::expected<audio_engine, error> {
    return audio_engine::try_make();
}
}  // namespace rin