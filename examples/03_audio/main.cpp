#include "rin/audio.hpp"
#include "rin/key.hpp"
#include "rin/window.hpp"

auto main() -> int {
    auto win_res = rin::try_make_window(800, 600);
    if (not win_res) win_res.error().panic();
    auto& win = *win_res;

    // audio_engineの生成, 失敗した場合は異常終了
    auto engine_res = rin::try_make_audio_engine();
    if (not engine_res) engine_res.error().panic();
    auto& engine = *engine_res;

    // soundの生成、失敗した場合は異常終了
    auto sound_res = engine.try_load_sound("audio.mp3");
    if (not sound_res) sound_res.error().panic();
    auto& sound = *sound_res;

    sound.play();  // 再生開始

    while (win.is_open()) {
        win.poll_events();
        if (win.is_key_pressed(rin::key_p)) {
            sound.looping(not sound.is_looping());  // Pキーが押された場合、ループを反転
        } else if (win.is_key_pressed(rin::key_up)) {
            sound.volume(sound.volume() + 10.f);  // upキーが押された場合、volumeを上げる
        } else if (win.is_key_pressed(rin::key_down)) {
            sound.volume(sound.volume() - 10.f);          // downキーが押された場合、下げる
        } else if (win.is_key_pressed(rin::key_space)) {  // スペースキーが押された場合
            if (sound.is_playing())
                sound.stop();  // 再生中であれば停止
            else
                sound.play();  // 停止中であれば再生
        }
    }
}