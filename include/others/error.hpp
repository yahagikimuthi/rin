#pragma once

#include <cstdlib>
#include <expected>
#include <iostream>
#include <ranges>
#include <string_view>
#include <variant>
#include <vector>

#include "others/type.hpp"
#include "others/util.hpp"

namespace rin {
enum class error_type : u8 { logic, runtime };
inline constexpr auto logic_error   = error_type::logic;
inline constexpr auto runtime_error = error_type::runtime;

struct error_code final {  // NOLINT
    using str_t = std::variant<std::string_view, std::string>;

    [[nodiscard]] auto message_to_str() const noexcept -> std::string {
        return message.visit([](auto&& str) noexcept -> std::string {
            return static_cast<std::string>(str);
        });
    }
    [[nodiscard]] auto message_to_view() const noexcept -> std::string_view {
        return message.visit([](auto&& str) noexcept -> std::string_view { return str; });
    }

    error_type type;
    str_t      message;
};

class [[nodiscard]] error final {
    friend constexpr auto make_error(
        const error_type type, const string_literal auto& message
    ) noexcept -> std::unexpected<error>;
    friend constexpr auto make_error(
        const error_type type, const string_literal auto& message, const error& child_error
    ) noexcept -> std::unexpected<error>;
    friend constexpr auto make_error(const error_type type, const std::string_view message) noexcept
        -> std::unexpected<error>;
    friend constexpr auto make_error(
        const error_type type, const std::string_view message, const error& child_error
    ) noexcept -> std::unexpected<error>;

  public:
    [[nodiscard]] auto type() const noexcept -> error_type { return codes_.front().type; }

    [[nodiscard]] auto message() const noexcept -> std::string {
        auto out = codes_ |
                   std::views::transform([](const error_code& code) noexcept -> std::string {
                       if (code.type == logic_error)
                           return "[Logic Error] " + code.message_to_str();
                       return "[Runtime Error]: " + code.message_to_str();
                   }) |
                   std::views::join_with(std::string{"\n -> "}) | std::ranges::to<std::string>();
        out += "\n";
        return out;
    }

    void what() const noexcept {
        for (const auto [i, code] : std::views::enumerate(codes_)) {
            if (i != 0) {
                std::cerr << " -> ";
            }
            if (code.type == logic_error) {
                std::cerr << "[Logic Error]: " << code.message_to_view() << '\n';
            } else {
                std::cerr << "[Runtime Error]: " << code.message_to_view() << '\n';
            }
        }
    }

    [[noreturn]] void panic() const noexcept {
        what();
        std::abort();
    }

  private:
    explicit error(std::vector<error_code>&& codes) noexcept : codes_{std::move(codes)} {}

    std::vector<error_code> codes_;
};

[[nodiscard]] constexpr auto make_error(
    const error_type type, const string_literal auto& message
) noexcept -> std::unexpected<error> {
    const auto code         = error_code{.type = type, .message = std::string_view{message}};
    auto       error_object = error{std::vector{code}};
    return std::unexpected{error_object};
}

[[nodiscard]] constexpr auto make_error(
    const error_type type, const string_literal auto& message, const error& child_error
) noexcept -> std::unexpected<error> {
    auto codes = std::vector<error_code>{};
    codes.reserve(1 + child_error.codes_.size());
    codes.emplace_back(type, std::string_view{message});
    codes.append_range(child_error.codes_);

    auto error_obj = error{std::move(codes)};
    return std::unexpected{error_obj};
}

[[nodiscard]] constexpr auto make_error(
    const error_type type, const std::string_view message
) noexcept -> std::unexpected<error> {
    const auto code      = error_code{.type = type, .message = std::string{message}};
    auto       error_obj = error{std::vector{code}};
    return std::unexpected{error_obj};
}

[[nodiscard]] constexpr auto make_error(
    const error_type type, const std::string_view message, const error& child_error
) noexcept -> std::unexpected<error> {
    auto codes = std::vector<error_code>{};
    codes.reserve(1 + child_error.codes_.size());
    codes.emplace_back(type, std::string{message});
    codes.append_range(child_error.codes_);

    auto error_obj = error{std::move(codes)};
    return std::unexpected{error_obj};
}
}  // namespace rin