#pragma once

#include <type_traits>

#include <variant>

namespace rin {
template <typename T, typename Variant>
struct is_variant_member_impl : std::false_type {};

template <typename T, typename... Ts>
struct is_variant_member_impl<T, std::variant<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> or ...)> {};

template <typename T, typename Variant>
concept variant_member = is_variant_member_impl<T, Variant>::value;
}  // namespace rin