#pragma once

#include <exception>
#include <expected>
#include <iostream>
#include <ranges>
#include <string_view>
#include <variant>
#include <vector>

#include "rin/detail/others.hpp"
#include "rin/types.hpp"

/**
 * @file error.hpp
 * @brief エラーの種類およびエラー情報を保持・追跡するエラーハンドリングモジュール
 */

namespace rin {
/**
 * @enum error_type
 * @brief エラーの種類を表す列挙体
 */
enum class error_type : u8 {
    logic,   ///< ロジックエラー（呼び出し側の事前条件違反など）
    runtime  ///< 実行時エラー（環境や入出力に起因する不具合など）
};

/// logic エラーの簡略エイリアス
inline constexpr auto logic_error = error_type::logic;
/// runtime エラーの簡略エイリアス
inline constexpr auto runtime_error = error_type::runtime;

class error final {
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

  public:
    /**
     * @brief 文字列リテラルからエラーインスタンス（std::unexpected）を生成します。
     *
     * @tparam T 文字列リテラルの型
     * @param type エラーの種類（logic または runtime）
     * @param message エラーメッセージ（文字列リテラル）
     * @return std::unexpected<error> エラーオブジェクトを保持する std::unexpected
     */
    template <typename T>
        requires detail::is_string_literal_v<T>
    [[nodiscard]] static auto make(const error_type type, const T& message) noexcept
        -> std::unexpected<error> {
        const auto codes = error_code{.type = type, .message = std::string_view{message}};
        const auto out   = error{std::vector{codes}};
        return std::unexpected{out};
    }

    /**
     * @brief 親エラーの文脈を含めて文字列リテラルからエラーインスタンスを生成します。
     *
     * @tparam T 文字列リテラルの型
     * @param type エラーの種類
     * @param message エラーメッセージ（文字列リテラル）
     * @param child_error 追記元となる既存のエラーオブジェクト
     * @return std::unexpected<error> エラーチェーンが連結された std::unexpected
     */
    template <typename T>
        requires detail::is_string_literal_v<T>
    [[nodiscard]] static auto make(
        const error_type type, const T& message, const error& child_error
    ) noexcept -> std::unexpected<error> {
        auto codes = std::vector<error_code>{};
        codes.reserve(1 + child_error.codes_.size());
        codes.emplace_back(type, std::string_view{message});
        codes.append_range(child_error.codes_);

        const auto out = error{std::move(codes)};
        return std::unexpected{out};
    }

    /**
     * @brief std::string_view からエラーインスタンス（std::unexpected）を生成します。
     *
     * @param type エラーの種類
     * @param message エラーメッセージ
     * @return std::unexpected<error> エラーオブジェクトを保持する std::unexpected
     */
    [[nodiscard]] static auto make(const error_type type, const std::string_view message) noexcept
        -> std::unexpected<error> {
        const auto codes = error_code{.type = type, .message = std::string{message}};
        const auto out   = error{std::vector{codes}};
        return std::unexpected{out};
    }

    /**
     * @brief 親エラーの文脈を含めて std::string_view からエラーインスタンスを生成します。
     *
     * @param type エラーの種類
     * @param message エラーメッセージ
     * @param child_error 追記元となる既存のエラーオブジェクト
     * @return std::unexpected<error> エラーチェーンが連結された std::unexpected
     */
    [[nodiscard]] static auto make(
        const error_type type, const std::string_view message, const error& child_error
    ) noexcept -> std::unexpected<error> {
        auto codes = std::vector<error_code>{};
        codes.reserve(1 + child_error.codes_.size());
        codes.emplace_back(type, std::string{message});
        codes.append_range(child_error.codes_);

        const auto out = error{std::move(codes)};
        return std::unexpected{out};
    }

    /**
     * @brief 最も根本（先頭）のエラー種別を取得します。
     *
     * @return error_type エラー種別
     */
    [[nodiscard]] auto type() const noexcept -> error_type { return codes_.front().type; }

    [[nodiscard]] auto what() const noexcept -> std::string {
        auto out = std::string{};
        for (const auto [i, code] : std::views::enumerate(codes_)) {
            if (i != 0) {
                out += " -> ";
            }
            if (code.type == logic_error) {
                out += "[Logic Error]: " + code.message_to_str() + '\n';
            } else {
                out += "[Runtime Error]: " + code.message_to_str() + '\n';
            }
        }
        return out;
    }

    /**
     * @brief 蓄積されたエラーチェーンを標準エラー出力（std::cerr）へ出力します。
     */
    void cerr() const noexcept {
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

    /**
     * @brief エラー内容を出力した上で、終了ハンドラを呼び出します。
     */
    [[noreturn]] void panic() const noexcept {
        cerr();
        std::terminate();
    }

  private:
    explicit error(std::vector<error_code>&& codes) noexcept : codes_{std::move(codes)} {}

    std::vector<error_code> codes_;
};

/**
 * @brief 文字列リテラルからエラーインスタンス（std::unexpected）を生成します。
 *
 * @tparam T 文字列リテラルの型
 * @param type エラーの種類（logic または runtime）
 * @param message エラーメッセージ（文字列リテラル）
 * @return std::unexpected<error> エラーオブジェクトを保持する std::unexpected
 */
template <typename T>
    requires detail::is_string_literal_v<T>
[[nodiscard]] constexpr auto make_error(const error_type type, const T& message) noexcept
    -> std::unexpected<error> {
    return error::make(type, message);
}

/**
 * @brief 親エラーの文脈を含めて文字列リテラルからエラーインスタンスを生成します。
 *
 * @tparam T 文字列リテラルの型
 * @param type エラーの種類
 * @param message エラーメッセージ（文字列リテラル）
 * @param child_error 追記元となる既存のエラーオブジェクト
 * @return std::unexpected<error> エラーチェーンが連結された std::unexpected
 */
template <typename T>
    requires detail::is_string_literal_v<T>
[[nodiscard]] constexpr auto make_error(
    const error_type type, const T& message, const error& child_error
) noexcept -> std::unexpected<error> {
    return error::make(type, message, child_error);
}

/**
 * @brief std::string_view からエラーインスタンス（std::unexpected）を生成します。
 *
 * @param type エラーの種類
 * @param message エラーメッセージ
 * @return std::unexpected<error> エラーオブジェクトを保持する std::unexpected
 */
[[nodiscard]] constexpr auto make_error(
    const error_type type, const std::string_view message
) noexcept -> std::unexpected<error> {
    return error::make(type, message);
}

/**
 * @brief 親エラーの文脈を含めて std::string_view からエラーインスタンスを生成します。
 *
 * @param type エラーの種類
 * @param message エラーメッセージ
 * @param child_error 追記元となる既存のエラーオブジェクト
 * @return std::unexpected<error> エラーチェーンが連結された std::unexpected
 */
[[nodiscard]] constexpr auto make_error(
    const error_type type, const std::string_view message, const error& child_error
) noexcept -> std::unexpected<error> {
    return error::make(type, message, child_error);
}
}  // namespace rin