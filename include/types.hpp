#pragma once

#include <cstdint>

/**
 * @file types.hpp
 * @brief 基本的な数値型エイリアスおよびサイズ保証を行うモジュール
 */

namespace rin {
/// 8ビット無符号整数型
using u8 = std::uint8_t;

/// 16ビット無符号整数型
using u16 = std::uint16_t;

/// 32ビット無符号整数型
using u32 = std::uint32_t;

/// 32ビット有符号整数型
using i32 = std::int32_t;

/// 32ビット単精度浮動小数点数型
/// @note `<stdfloat>`（std::float32_t 等）の環境依存・未安定性を考慮し、標準の `float`
/// をエイリアスとして採用しています。
using f32 = float;

/// 64ビット倍精度浮動小数点数型
/// @note `<stdfloat>`（std::float64_t 等）の環境依存・未安定性を考慮し、標準の `double`
/// をエイリアスとして採用しています。
using f64 = double;

static_assert(sizeof(float) == 4, "We expect the float to be 32bits");
static_assert(sizeof(double) == 8, "We expect the double to be 64bits");
}  // namespace rin