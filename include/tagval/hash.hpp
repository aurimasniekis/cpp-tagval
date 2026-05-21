#pragma once

/// @file
/// @brief std::hash partial specialization for any tagval value type.
///
/// Hashes the (kind_id, code) pair so cross-kind handles do not collide
/// accidentally. Hash values are not stable across processes — the underlying
/// std::hash<std::string_view> is implementation-defined and may even be
/// salted per-process.

#include <tagval/base.hpp>

#include <concepts>
#include <cstddef>
#include <functional>
#include <string_view>

namespace tagval::detail {

[[nodiscard]] inline std::size_t hash_combine(std::size_t seed, const std::size_t v) noexcept {
    // boost::hash_combine constants — well-distributed mixing for 64-bit seeds.
    seed ^= v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    return seed;
}

}  // namespace tagval::detail

template <typename T>
    requires std::derived_from<T, tagval::detail::TagValBaseTag>
struct std::hash<T> {  // NOLINT(cert-dcl58-cpp)
    [[nodiscard]] std::size_t operator()(const T& v) const noexcept {
        std::size_t s = std::hash<std::string_view>{}(T::kind_id());
        s = tagval::detail::hash_combine(s, std::hash<std::string_view>{}(v.code()));
        return s;
    }
};
