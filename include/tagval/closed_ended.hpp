#pragma once

/// @file
/// @brief ClosedEnded base — values come from a compile-time `Values<...>` list.
///
/// Required Derived shape:
///   * Derives from `tagval::ClosedEnded<"id", Derived>`.
///   * Declares `using values_t = tagval::Values<E1, E2, ...>` listing every
///     entry; each E_i must be a `tagval::Entry` whose owner_t is Derived.
///   * Each entry is typically declared in-class with TAGVAL_ENTRY or
///     TAGVAL_ENTRY_AS so the per-entry accessor is generated automatically.

#include <tagval/base.hpp>
#include <tagval/entry.hpp>
#include <tagval/open_ended.hpp>  // for detail::HasValuesT
#include <tagval/values.hpp>

#include <commons/fixed_string.hpp>

#include <expected>
#include <span>
#include <string_view>

namespace tagval {

/// CRTP base for tag-value kinds whose values are fixed at compile time.
///
///     class Status : public tagval::ClosedEnded<"status", Status> {
///     public:
///         using base_t = tagval::ClosedEnded<"status", Status>;
///         using base_t::base_t;
///         TAGVAL_ENTRY(Status, Active,   active);
///         TAGVAL_ENTRY(Status, Inactive, inactive, "Inactive");
///         using values_t = tagval::Values<Active, Inactive>;
///     };
template <comms::FixedString Id, typename Derived>
class ClosedEnded : public detail::HandleBase<Id, Derived> {
    using base_t = detail::HandleBase<Id, Derived>;

public:
    using base_t::base_t;

    /// Compile-time canonical handle for an entry. Static-asserts the entry is
    /// listed in Derived::values_t. The handle's address is the address of the
    /// matching record inside all_values(), so it compares equal to the result
    /// of of(E::code).
    template <typename E>
    [[nodiscard]] static const Derived& value() noexcept {
        static_assert(detail::HasValuesT<Derived>,
                      "tagval: kind type is missing `using values_t = tagval::Values<...>;`");
        static_assert(Derived::values_t::template contains<E>,
                      "Entry type is not listed in Derived::values_t");
        static const Derived h = base_t::make_handle(base_t::find_in(all_values(), E::code));
        return h;
    }

    /// All values for this kind, in declaration order. Span points into a
    /// constexpr static array — valid for the program's lifetime.
    [[nodiscard]] static std::span<const TagValMetadata> all_values() noexcept {
        static_assert(detail::HasValuesT<Derived>,
                      "tagval: kind type is missing `using values_t = tagval::Values<...>;`");
        static constexpr auto arr = values_metadata_array<typename Derived::values_t>();
        return std::span<const TagValMetadata>{arr};
    }

    /// Look up a value by code. Throws UnknownCodeError on miss.
    [[nodiscard]] static Derived of(std::string_view code) {
        return base_t::of_impl(all_values(), code);
    }

    /// Look up a value by code. Returns ParseError on miss.
    [[nodiscard]] static std::expected<Derived, ParseError> try_of(std::string_view code) noexcept {
        return base_t::try_of_impl(all_values(), code);
    }
};

}  // namespace tagval
