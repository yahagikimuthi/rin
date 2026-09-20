#pragma once

#include <concepts>
#include <iostream>
#include <string_view>

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

class Error final {
  public:
    enum class Type : u8 { logic, runtime };

    explicit Error(const Type type) noexcept : type_{type} {}
    explicit Error(const Type type, const std::string_view message)
        : type_{type}, message_{message} {}

    [[nodiscard]] auto type() const noexcept -> Type { return type_; }

    [[nodiscard]] auto message() const noexcept -> std::string_view { return message_; }

    void what() const noexcept {
        if (type_ == Type::logic) {
            std::cerr << "[Logic Error]\n";
        } else {
            std::cerr << "[Runtime Error]\n";
        }
        std::cerr << message_ << '\n';
    }

  private:
    const Type             type_;
    const std::string_view message_;
};
}  // namespace rin