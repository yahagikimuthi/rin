module;

#include "rin/audio.hpp"
#include "rin/clock.hpp"
#include "rin/collision.hpp"
#include "rin/color.hpp"
#include "rin/error.hpp"
#include "rin/extent.hpp"
#include "rin/font.hpp"

export module rin;

export namespace rin {
// sound.hpp
using rin::audio_engine;
using rin::sound;
using rin::try_make_audio_engine;

// clock.hpp
using rin::clock;
using rin::make_clock;

// collision.hpp
using rin::aabb_bound;
using rin::circle_bound;
using rin::intersects;
using rin::make_aabb_bound;
using rin::make_circle_bound;

// color.hpp
using rin::aqua;
using rin::black;
using rin::blue;
using rin::fuchsia;
using rin::gray;
using rin::green;
using rin::lime;
using rin::maroon;
using rin::navy;
using rin::olive;
using rin::purple;
using rin::red;
using rin::rgba;
using rin::silver;
using rin::teal;
using rin::white;
using rin::yellow;

// error.hpp
using rin::error;
using rin::error_type;
using rin::logic_error;
using rin::make_error;
using rin::runtime_error;

// extent.hpp
using rin::extent;

// font.hpp
using rin::font;
using rin::try_make_font;
}  // namespace rin