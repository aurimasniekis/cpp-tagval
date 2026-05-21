#pragma once

/// @file
/// @brief OpenEnded base — same in-class declaration shape as ClosedEnded
///        (TAGVAL_ENTRY + `using values_t = tagval::Values<...>`) but
///        additionally accepts plugin entries declared at namespace scope
///        with TAGVAL_EXTERN_ENTRY, which register into
///        OpenEndedRegistry<Owner> at static-init.
///
/// Required Derived shape:
///   * Derives from `tagval::OpenEnded<"id", Derived>`.
///   * Optionally declares `using values_t = tagval::Values<...>` — the
///     compile-time portion of the value set.
///   * May contribute extern entries via TAGVAL_EXTERN_ENTRY[_AS] at namespace
///     scope.
///
/// Storage model: every entry contributes a pointer to a pinned
/// `tagval::metadata_v<E>` constant. OpenEndedRegistry<Derived> stores these pointers
/// (not copies); handles point at the same pinned record everywhere, so
/// pointer equality matches across `value<E>()`, `of(code)`, and `try_of(code)`
/// regardless of which translation unit produced the entry.

#include <tagval/base.hpp>
#include <tagval/entry.hpp>
#include <tagval/error.hpp>
#include <tagval/fixed_string.hpp>
#include <tagval/openended_registry.hpp>
#include <tagval/values.hpp>

#include <concepts>
#include <expected>
#include <ranges>
#include <string>
#include <string_view>

namespace tagval {

namespace detail {

template <typename T>
concept HasValuesT = requires { typename T::values_t; };

}  // namespace detail

/// CRTP base for tag-value kinds whose values are partly compile-time
/// (Derived::values_t) and partly contributed by plugins via
/// TAGVAL_EXTERN_ENTRY.
///
///     class Kind : public tagval::OpenEnded<"kind", Kind> {
///     public:
///         using base_t = tagval::OpenEnded<"kind", Kind>;
///         using base_t::base_t;
///         TAGVAL_ENTRY(Kind, Phone,  phone,  "Phone")
///         TAGVAL_ENTRY(Kind, Tablet, tablet, "Tablet")
///         using values_t = tagval::Values<Phone, Tablet>;
///     };
///
///     namespace vendor {
///     TAGVAL_EXTERN_ENTRY(::Kind, SmartWatch, smart_watch, "Smart Watch");
///     }
template <fixed_string Id, typename Derived>
class OpenEnded : public detail::HandleBase<Id, Derived> {
    using base_t = detail::HandleBase<Id, Derived>;

    /// Idempotently seeds OpenEndedRegistry<Self> with the pinned pointers from
    /// `Self::values_t` the first time any entry-point below references it.
    /// OpenEndedRegistry::add dedups by code, so racing with an extern registrar that
    /// happens to use the same code is harmless.
    template <typename Self = Derived>
    // NOLINTNEXTLINE(readability-identifier-naming)
    inline static const bool values_t_seeded_ = [] {
        if constexpr (detail::HasValuesT<Self>) {
            for (constexpr auto ptrs = values_metadata_pointers<typename Self::values_t>();
                 const auto* p : ptrs) {
                OpenEndedRegistry<Self>::add(p);
            }
        }
        return true;
    }();

public:
    using base_t::base_t;

    /// Canonical handle for an entry. Looked up via OpenEndedRegistry, so the handle's
    /// address agrees with `of(E::code)` whether E came from values_t or from
    /// a TAGVAL_EXTERN_ENTRY in another translation unit.
    template <typename E>
    [[nodiscard]] static const Derived& value() noexcept {
        static_assert(std::same_as<typename E::owner_t, Derived>,
                      "Entry type's owner must match this kind");
        (void)values_t_seeded_<Derived>;
        static const Derived h = base_t::make_handle(OpenEndedRegistry<Derived>::find(E::code));
        return h;
    }

    /// Range of TagValMetadata covering compile-time `values_t` entries
    /// (seeded lazily on first call) plus extern entries currently registered
    /// in OpenEndedRegistry<Derived>. Each element is a reference to the pinned
    /// metadata storage and remains valid for the program's lifetime.
    [[nodiscard]] static auto all_values() noexcept {
        (void)values_t_seeded_<Derived>;
        return OpenEndedRegistry<Derived>::all() |
               std::views::transform(
                   [](const TagValMetadata* p) -> const TagValMetadata& { return *p; });
    }

    /// Look up a value by code. Throws UnknownCodeError on miss.
    [[nodiscard]] static Derived of(std::string_view code) {
        (void)values_t_seeded_<Derived>;
        const TagValMetadata* m = OpenEndedRegistry<Derived>::find(code);
        if (m == nullptr) {
            std::string msg = "tagval: unknown code '";
            msg.append(code);
            msg += "' for kind '";
            msg.append(Id.view());
            msg += "'";
            throw UnknownCodeError{msg};
        }
        return base_t::make_handle(m);
    }

    /// Look up a value by code. Returns ParseError on miss.
    [[nodiscard]] static std::expected<Derived, ParseError> try_of(std::string_view code) noexcept {
        (void)values_t_seeded_<Derived>;
        const TagValMetadata* m = OpenEndedRegistry<Derived>::find(code);
        if (m == nullptr) {
            return std::unexpected(ParseError{.code = std::string{code}, .kind_id = Id.view()});
        }
        return base_t::make_handle(m);
    }
};

}  // namespace tagval
