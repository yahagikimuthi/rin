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

    explicit Error(const Type type, std::vector<std::string_view>&& messages) noexcept
        : type_{type}, messages_{std::move(messages)} {}

    template <typename... Errors>
        requires(std::is_same_v<Errors, Error> and ...)
    [[nodiscard]] static constexpr auto create(
        const Type type, std::string_view message, Errors&... other_errors
    ) noexcept -> std::unexpected<Error> {
        auto messages = std::vector<std::string_view>{};

        auto message_cnt = std::size_t{1};
        (add_error_message_cnt(other_errors, message_cnt), ...);
        messages.reserve(message_cnt);

        messages.emplace_back(message);
        (add_error_message(other_errors, messages), ...);

        return std::unexpected<Error>{std::in_place, type, std::move(messages)};
    }

    [[nodiscard]] auto type() const noexcept -> Type { return type_; }
    [[nodiscard]] auto message() const noexcept -> std::string {
        const auto out = std::format("{}", std::views::join_with(messages_, " -> "));
        return out;
    }
    void what() const noexcept {
        if (type_ == Type::logic)
            std::cerr << "[Logic Error]\n";
        else
            std::cerr << "[Runtime Error]\n";

        for (const auto [i, message] : std::views::enumerate(messages_)) {
            if (i == 0)
                std::cerr << message << '\n';
            else
                std::cerr << " -> " << message << '\n';
        }
    }

  private:
    static void add_error_message_cnt(const Error& error, std::size_t& add_var) noexcept {
        add_var += error.messages_.size();
    }
    static void add_error_message(
        const Error& error, std::vector<std::string_view>& add_vec
    ) noexcept {
        for (const auto message : error.messages_) add_vec.emplace_back(message);
    }

    Type                          type_;
    std::vector<std::string_view> messages_;
};
}  // namespace rin