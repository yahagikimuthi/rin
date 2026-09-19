#pragma once

#include <concepts>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include "type.hpp"

namespace rin {
[[nodiscard]] inline auto read_file(const std::string_view path) noexcept
    -> std::optional<std::string> {
    auto file = std::ifstream{std::string{path}, std::ios::in | std::ios::binary};

    if (not file.is_open()) return std::nullopt;

    auto stream = std::stringstream{};
    stream << file.rdbuf();
    return stream.str();
}

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
}  // namespace rin