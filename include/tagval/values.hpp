#pragma once

/// @file
/// @brief Compile-time list of Entry types for ClosedEnded kinds.

#include <tagval/entry.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <string_view>
#include <tuple>

namespace tagval::detail {

template <typename... Es>
[[nodiscard]] consteval bool all_codes_unique() noexcept {
    if constexpr (sizeof...(Es) <= 1) {
        return true;
    } else {
        const std::array<std::string_view, sizeof...(Es)> codes{std::string_view{Es::code}...};
        for (std::size_t i = 0; i < codes.size(); ++i) {
            for (std::size_t j = i + 1; j < codes.size(); ++j) {
                if (codes[i] == codes[j]) {
                    return false;
                }
            }
        }
        return true;
    }
}

}  // namespace tagval::detail

namespace tagval {

/// Compile-time tuple of Entry types belonging to a single Owner.
///
///     using values_t = tagval::Values<Active, Inactive, Archived>;
template <typename... Entries>
struct Values {
    static_assert(sizeof...(Entries) > 0, "Values<> must list at least one entry");

    using entries_t = std::tuple<Entries...>;
    using owner_t = std::tuple_element_t<0, entries_t>::owner_t;

    static_assert((std::same_as<typename Entries::owner_t, owner_t> && ...),
                  "All entries in tagval::Values must share the same owner");

    static_assert(detail::all_codes_unique<Entries...>(),
                  "tagval::Values: duplicate code in entry list");

    template <typename E>
    static constexpr bool contains = (std::same_as<E, Entries> || ...);

    static constexpr std::size_t size = sizeof...(Entries);
};

namespace detail {

template <typename... Es>
[[nodiscard]] constexpr auto values_metadata_array_impl(std::tuple<Es...>* /*tag*/) {
    return std::array<TagValMetadata, sizeof...(Es)>{metadata_of<Es>()...};
}

template <typename... Es>
[[nodiscard]] constexpr auto values_metadata_pointers_impl(std::tuple<Es...>* /*tag*/) {
    return std::array<const TagValMetadata*, sizeof...(Es)>{&metadata_v<Es>...};
}

}  // namespace detail

/// Materialize the metadata array for a `Values<...>` list. Used by
/// ClosedEnded::all_values() to expose a constexpr metadata snapshot.
template <typename V>
[[nodiscard]] constexpr auto values_metadata_array() {
    return detail::values_metadata_array_impl(static_cast<V::entries_t*>(nullptr));
}

/// Materialize an array of pointers to the pinned `metadata_v<E>` constants for
/// a `Values<...>` list. Used by OpenEnded to seed Registry with stable
/// addresses identical to those used by extern entries.
template <typename V>
[[nodiscard]] constexpr auto values_metadata_pointers() {
    return detail::values_metadata_pointers_impl(static_cast<V::entries_t*>(nullptr));
}

}  // namespace tagval
