#pragma once

#include <cstddef>
#include <expected>
#include <format>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

#include "type.hpp"

namespace rin {
class Error final {
  public:
    enum class Type : u8 { logic, runtime };

    using Type::logic;
    using Type::runtime;

  private:
    struct Code final {
        Type             type;
        std::string_view message;
    };

  public:
    [[nodiscard]] static auto create(const Type type, const std::string_view message) noexcept
        -> std::unexpected<Error> {
        const auto code  = Code{.type = type, .message = message};
        auto       error = Error{std::vector{code}};
        return std::unexpected{error};
    }

    [[nodiscard]] static auto create(
        const Type type, const std::string_view message, const Error& child_error
    ) noexcept -> std::unexpected<Error> {
        auto codes = std::vector<Code>{};
        codes.reserve(1 + child_error.error_codes_.size());
        codes.emplace_back(type, message);
        codes.append_range(child_error.error_codes_);

        auto error = Error{std::move(codes)};
        return std::unexpected{error};
    }

    [[nodiscard]] auto type() const noexcept -> Type { return error_codes_.front().type; }
    [[nodiscard]] auto message() const noexcept -> std::string {
        const auto out = std::format(
            "{}",
            std::views::join_with(
                error_codes_ | std::views::transform([](const Code& code) -> std::string {
                    if (code.type == logic) return "[Logic Error]: " + std::string{code.message};

                    return "[Runtime Error]: " + std::string{code.message};
                }),
                " -> "
            )
        );
        return out;
    }
    void what() const noexcept {
        for (const auto [i, code] : std::views::enumerate(error_codes_)) {
            if (i != 0) {
                std::cerr << " -> ";
            }
            if (code.type == logic) {
                std::cerr << "[Logic Error]: " << code.message << '\n';
            } else {
                std::cerr << "[Runtime Error]: " << code.message << '\n';
            }
        }
    }

  private:
    explicit Error(std::vector<Code>&& codes) noexcept : error_codes_{std::move(codes)} {}

    std::vector<Code> error_codes_;
};
}  // namespace rin