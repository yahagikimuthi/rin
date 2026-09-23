#pragma once

#include <expected>
#include <format>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

#include "type.hpp"

namespace rin {
enum class error_type : u8 { logic, runtime };
inline constexpr auto logic_error   = error_type::logic;
inline constexpr auto runtime_error = error_type::runtime;

class [[nodiscard]] error final {
  private:
    struct error_code final {
        error_type       type;
        std::string_view message;
    };

  public:
    [[nodiscard]] static auto create(const error_type type, const std::string_view message) noexcept
        -> std::unexpected<error> {
        const auto code         = error_code{.type = type, .message = message};
        auto       error_object = error{std::vector{code}};
        return std::unexpected{error_object};
    }

    [[nodiscard]] static auto create(
        const error_type type, const std::string_view message, const error& child_error
    ) noexcept -> std::unexpected<error> {
        auto codes = std::vector<error_code>{};
        codes.reserve(1 + child_error.codes_.size());
        codes.emplace_back(type, message);
        codes.append_range(child_error.codes_);

        auto error_object = error{std::move(codes)};
        return std::unexpected{error_object};
    }

    [[nodiscard]] auto type() const noexcept -> error_type { return codes_.front().type; }

    [[nodiscard]] auto message() const noexcept -> std::string {
        const auto out = std::format(
            "{}",
            std::views::join_with(
                codes_ | std::views::transform([](const error_code& error_code) -> std::string {
                    if (error_code.type == logic_error)
                        return "[Logic Error]: " + std::string{error_code.message};

                    return "[Runtime Error]: " + std::string{error_code.message};
                }),
                "\n -> "
            )
        );
        return out;
    }

    void what() const noexcept {
        for (const auto [i, code] : std::views::enumerate(codes_)) {
            if (i != 0) {
                std::cerr << " -> ";
            }
            if (code.type == logic_error) {
                std::cerr << "[Logic Error]: " << code.message << '\n';
            } else {
                std::cerr << "[Runtime Error]: " << code.message << '\n';
            }
        }
    }

  private:
    explicit error(std::vector<error_code>&& codes) noexcept : codes_{std::move(codes)} {}

    std::vector<error_code> codes_;
};
}  // namespace rin