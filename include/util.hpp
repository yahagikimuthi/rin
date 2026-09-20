#pragma once

#include <concepts>
#include <expected>
#include <iostream>
#include <string_view>
#include <utility>

#include "type.hpp"

namespace rin {

using namespace std::string_view_literals;

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

    constexpr Error(const Type type, const std::string_view message = ""sv)
        : type_{type}, message_{message} {}

    [[nodiscard]] constexpr auto type() const noexcept -> Type { return type_; }

    [[nodiscard]] constexpr auto message() const noexcept -> std::string_view { return message_; }

    [[nodiscard]] static constexpr auto create(
        const Type type, const std::string_view message = ""sv
    ) noexcept -> std::unexpected<Error> {
        return std::unexpected<Error>{std::in_place, type, message};
    }

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