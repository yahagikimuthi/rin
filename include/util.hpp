#pragma once

#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace rin {
[[nodiscard]] inline auto read_file(const std::string_view path) noexcept
    -> std::optional<std::string> {
    auto file = std::ifstream{std::string{path}, std::ios::in | std::ios::binary};

    if (not file.is_open()) return std::nullopt;

    auto stream = std::stringstream{};
    stream << file.rdbuf();
    return stream.str();
}
}  // namespace rin