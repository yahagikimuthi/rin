#pragma once

#include <concepts>

#include "type.hpp"

namespace rin {
template <typename T>
concept Numeric = std::integral<T> or std::floating_point<T>;

template <Numeric T>
struct Vector2 final {
    T x;
    T y;
};

using Vector2f = Vector2<f32>;

struct RGB final {
    f32 r;
    f32 g;
    f32 b;
};

struct UV final {
    f32 u;
    f32 v;
};
}  // namespace rin