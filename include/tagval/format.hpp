#pragma once

/// @file
/// @brief std::formatter partial specialization — formats any tagval value as its code().
///
/// Supports only the empty format spec ("{}"). The output is `value.code()`.

#include <tagval/base.hpp>

#include <concepts>
#include <format>
#include <ranges>

template <typename T>
    requires std::derived_from<T, tagval::detail::TagValBaseTag>
struct std::formatter<T, char> {  // NOLINT(cert-dcl58-cpp)
    static constexpr auto parse(const std::format_parse_context& ctx) {
        const auto* it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error{"tagval: only the default format spec is supported"};
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const T& v, FormatContext& ctx) const {
        const auto code = v.code();
        return std::ranges::copy(code, ctx.out()).out;
    }
};
