#pragma once

/// @file
/// @brief TAGVAL_* macros for declaring entries.
///
/// In-class macros (`TAGVAL_ENTRY`, `TAGVAL_ENTRY_AS`) live inside the kind
/// class body and declare a nested Entry type plus a static accessor. The kind
/// must list these entries in `values_t` so they are visible to `all_values()`
/// (ClosedEnded) or to the compile-time portion of an OpenEnded merge.
///
/// External macros (`TAGVAL_EXTERN_ENTRY`, `TAGVAL_EXTERN_ENTRY_AS`) live at
/// namespace scope and always register into `OpenEndedRegistry<Owner>` —
/// there is no other reason to declare an entry outside its class.

#include <tagval/entry.hpp>
#include <tagval/kind_registry.hpp>
#include <tagval/openended_registry.hpp>

// `[[gnu::used]]` keeps the inline registrar variable in the final binary even
// when nothing else in its translation unit is referenced — required for extern
// entries built into a static archive. GCC and Clang (including Apple Clang)
// honour the attribute; MSVC needs `/INCLUDE:<mangled>` on the linker line and
// is not handled here (CI does not exercise MSVC for v0.1.0).
#if defined(__GNUC__) || defined(__clang__)
#define TAGVAL_DETAIL_USED [[gnu::used]]
#else
#define TAGVAL_DETAIL_USED
#endif

// In-class entry whose code is derived from the function name.
//
//     TAGVAL_ENTRY(Status, Active, active);
//     TAGVAL_ENTRY(Status, Inactive, inactive, "Inactive", "mdi:off");
#define TAGVAL_ENTRY(Owner, TypeName, FuncName, ...)                                               \
    struct TypeName : ::tagval::Entry<Owner, #FuncName __VA_OPT__(, ) __VA_ARGS__> {};             \
    [[maybe_unused]] [[nodiscard]] static auto const& FuncName() {                                 \
        return base_t::template value<TypeName>();                                                 \
    }

// In-class entry with explicit code:
//
//     TAGVAL_ENTRY_AS(Status, Archived, is_archived, "archived", "Archived");
#define TAGVAL_ENTRY_AS(Owner, TypeName, FuncName, Code, ...)                                      \
    struct TypeName : ::tagval::Entry<Owner, Code __VA_OPT__(, ) __VA_ARGS__> {};                  \
    [[maybe_unused]] [[nodiscard]] static auto const& FuncName() {                                 \
        return base_t::template value<TypeName>();                                                 \
    }

// External entry: declared at namespace scope, always registers.
#define TAGVAL_EXTERN_ENTRY(Owner, TypeName, FuncName, ...)                                        \
    struct TypeName : ::tagval::Entry<Owner, #FuncName __VA_OPT__(, ) __VA_ARGS__> {};             \
    [[maybe_unused]] [[nodiscard]] inline auto const& FuncName() {                                 \
        return Owner::template value<TypeName>();                                                  \
    }                                                                                              \
    /* NOLINTNEXTLINE(readability-identifier-naming) */                                            \
    TAGVAL_DETAIL_USED [[maybe_unused]] inline const bool TypeName##_registered_ = [] {            \
        ::tagval::OpenEndedRegistry<Owner>::add(&::tagval::metadata_v<TypeName>);                  \
        return true;                                                                               \
    }()

#define TAGVAL_EXTERN_ENTRY_AS(Owner, TypeName, FuncName, Code, ...)                               \
    struct TypeName : ::tagval::Entry<Owner, Code __VA_OPT__(, ) __VA_ARGS__> {};                  \
    [[maybe_unused]] [[nodiscard]] inline auto const& FuncName() {                                 \
        return Owner::template value<TypeName>();                                                  \
    }                                                                                              \
    /* NOLINTNEXTLINE(readability-identifier-naming) */                                            \
    TAGVAL_DETAIL_USED [[maybe_unused]] inline const bool TypeName##_registered_ = [] {            \
        ::tagval::OpenEndedRegistry<Owner>::add(&::tagval::metadata_v<TypeName>);                  \
        return true;                                                                               \
    }()

// Internal helpers for token pasting that survives one round of macro
// expansion — needed so __LINE__ in TAGVAL_REGISTER_KIND actually expands
// before the ##. Don't use directly.
#define TAGVAL_DETAIL_PASTE2(a, b) a##b
#define TAGVAL_DETAIL_PASTE(a, b) TAGVAL_DETAIL_PASTE2(a, b)

// Register a kind in the program-wide tagval::KindRegistry. Place at
// namespace scope (typically immediately after the kind's class definition,
// in either a header or a .cpp). Opt-in: kinds that are never registered do
// not appear in KindRegistry::all(). Idempotent on the kind's id; the same
// macro placed in multiple TUs is safe.
//
//     TAGVAL_REGISTER_KIND(Status);
//     TAGVAL_REGISTER_KIND(::vendor::PaymentMethod);
//
// The kind argument may be fully qualified — the registrar variable name is
// generated from __LINE__ so namespace-qualified types work directly without
// an extra alias. (`__COUNTER__` would also work but is flagged as a C2y
// extension under -Wc2y-extensions.) Limitation: two TAGVAL_REGISTER_KIND
// invocations on the same line of the same TU will collide — keep them on
// separate lines.
//
// Inherits the static-archive caveat from TAGVAL_EXTERN_ENTRY: if the macro
// lands in a TU that's only inside a .a and nothing else in that TU is
// referenced from the consumer, the registrar may be dead-stripped. Same
// workarounds apply (CMake OBJECT library, --whole-archive, -force_load).
#define TAGVAL_REGISTER_KIND(KindType)                                                             \
    /* NOLINTNEXTLINE(readability-identifier-naming) */                                            \
    TAGVAL_DETAIL_USED [[maybe_unused]] inline const bool TAGVAL_DETAIL_PASTE(                     \
        tagval_kind_registered_, __LINE__) = [] {                                                  \
        ::tagval::KindRegistry::register_kind<KindType>();                                         \
        return true;                                                                               \
    }()
