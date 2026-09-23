#pragma once

#include <expected>
#include <format>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

#include "type.hpp"

namespace rin {
class error final {
  public:
    enum class type : u8 { logic, runtime };

    using type::logic;
    using type::runtime;

  private:
    struct code final {
        type             error_type;
        std::string_view message;
    };

  public:
    [[nodiscard]] static auto create(const type error_type, const std::string_view message) noexcept
        -> std::unexpected<error> {
        const auto error_code   = code{.error_type = error_type, .message = message};
        auto       error_object = error{std::vector{error_code}};
        return std::unexpected{error_object};
    }

    [[nodiscard]] static auto create(
        const type error_type, const std::string_view message, const error& child_error
    ) noexcept -> std::unexpected<error> {
        auto codes = std::vector<code>{};
        codes.reserve(1 + child_error.error_codes_.size());
        codes.emplace_back(error_type, message);
        codes.append_range(child_error.error_codes_);

        auto error_object = error{std::move(codes)};
        return std::unexpected{error_object};
    }

    [[nodiscard]] auto type() const noexcept -> type { return error_codes_.front().error_type; }
    [[nodiscard]] auto message() const noexcept -> std::string {
        const auto out = std::format(
            "{}",
            std::views::join_with(
                error_codes_ | std::views::transform([](const code& error_code) -> std::string {
                    if (error_code.error_type == logic)
                        return "[Logic Error]: " + std::string{error_code.message};

                    return "[Runtime Error]: " + std::string{error_code.message};
                }),
                "\n -> "
            )
        );
        return out;
    }
    void what() const noexcept {
        for (const auto [i, error_code] : std::views::enumerate(error_codes_)) {
            if (i != 0) {
                std::cerr << " -> ";
            }
            if (error_code.error_type == logic) {
                std::cerr << "[Logic Error]: " << error_code.message << '\n';
            } else {
                std::cerr << "[Runtime Error]: " << error_code.message << '\n';
            }
        }
    }

  private:
    explicit error(std::vector<code>&& codes) noexcept : error_codes_{std::move(codes)} {}

    std::vector<code> error_codes_;
};
}  // namespace rin