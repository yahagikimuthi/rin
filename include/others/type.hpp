#pragma once

#include <cstdint>

namespace rin {
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using f32 = float;
using f64 = double;

static_assert(sizeof(float) == 4, "We expect that size of float is 32bit");
static_assert(sizeof(double) == 8, "We expect that size of double is 64bit");
}  // namespace rin