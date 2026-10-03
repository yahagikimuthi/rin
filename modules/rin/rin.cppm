module;

#include "rin/audio.hpp"
#include "rin/clock.hpp"
#include "rin/collision.hpp"
#include "rin/color.hpp"
#include "rin/error.hpp"
#include "rin/extent.hpp"
#include "rin/font.hpp"
#include "rin/sprite.hpp"
#include "rin/text.hpp"
#include "rin/texture.hpp"
#include "rin/types.hpp"
#include "rin/uv.hpp"
#include "rin/vec2.hpp"
#include "rin/vertex_vector.hpp"
#include "rin/window.hpp"

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
using rin::default_font_size;
using rin::font;
using rin::glyph;
using rin::try_make_font;

// sprite.hpp
using rin::make_sprite;
using rin::sprite;

// text.hpp
using rin::make_text;
using rin::text;

// texture.hpp
using rin::make_texture;
using rin::texture;
using rin::try_make_texture;
using rin::uv_rectangle;

// types.hpp
using rin::f32;
using rin::f64;
using rin::i16;
using rin::i32;
using rin::i64;
using rin::i8;
using rin::u16;
using rin::u32;
using rin::u64;
using rin::u8;

// uv.hpp
using rin::uv;
using rin::vec2;

// vertex_vector.hpp
using rin::primitive_line_strip;
using rin::primitive_lines;
using rin::primitive_points;
using rin::primitive_triangle_fan;
using rin::primitive_triangle_strip;
using rin::primitive_triangles;
using rin::primitive_type;
using rin::vertex;
using rin::vertex_vector;

// window.hpp
using rin::try_make_window;
using rin::window;
}  // namespace rin