#pragma once

/// @file
/// @brief NTTP-friendly fixed-size string used as a non-type template parameter.

#include <cstddef>
#include <string_view>

namespace tagval {

/// Compile-time fixed-size string usable as a non-type template parameter.
///
/// Construct via CTAD from a string literal:
///
///     fixed_string id{"device_kind"};      // N = 12 (includes null terminator)
///
/// The class is a structural type so it can appear directly in template
/// argument lists, e.g. `Entry<Owner, "active">`.
template <std::size_t N>
struct fixed_string {  // NOLINT(readability-identifier-naming)
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays,google-explicit-constructor,hicpp-explicit-conversions)
    char value[N]{};

    /// Implicit by design: enables `Entry<T, "foo">` literal syntax.
    constexpr fixed_string(const char (&str)[N]) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            value[i] = str[i];
        }
    }

    [[nodiscard]] constexpr std::string_view view() const noexcept {
        return std::string_view{static_cast<const char*>(value), N - 1};
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept {
        return view();
    }
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays,google-explicit-constructor,hicpp-explicit-conversions)

    [[nodiscard]] static constexpr std::size_t size() noexcept {
        return N - 1;
    }

    [[nodiscard]] static constexpr bool empty() noexcept {
        return N <= 1;
    }
};

}  // namespace tagval
