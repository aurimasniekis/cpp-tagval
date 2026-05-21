#pragma once

/// @file
/// @brief Compile-time Entry NTTP and the runtime-view TagValMetadata struct.
///
/// Each user-declared `struct Active : tagval::Entry<Status, "active", ...> {}`
/// is a distinct type. Per-kind storage (a `static constexpr` array for
/// ClosedEnded, a Meyers-singleton vector for OpenEnded) materializes a
/// `TagValMetadata` for every entry; handles store the address of the matching
/// record inside that storage. Pointer equality on that address is value
/// equality.

#include <tagval/fixed_string.hpp>

#include <string_view>

namespace tagval {

/// Compile-time entry. Subclass to declare a value:
///
///     struct Active : tagval::Entry<Status, "active", "Active",
///                                   "mdi:on", "#00ff00"> {};
///
/// Trailing metadata fields default to empty fixed_strings.
template <typename Owner,
          fixed_string Code,
          fixed_string Label = "",
          fixed_string Icon = "",
          fixed_string Color = "">
struct Entry {
    static_assert(Code.size() > 0, "tagval::Entry: code must not be empty");

    using owner_t = Owner;
    static constexpr auto code = Code;
    static constexpr auto label = Label;
    static constexpr auto icon = Icon;
    static constexpr auto color = Color;
};

/// Runtime view of an entry's metadata. The string_views point into the
/// eternal NTTP storage of the originating Entry type, so they remain valid
/// for the lifetime of the program.
struct TagValMetadata {
    std::string_view code;
    std::string_view label;
    std::string_view icon;
    std::string_view color;

    [[nodiscard]] friend constexpr bool operator==(const TagValMetadata&,
                                                   const TagValMetadata&) noexcept = default;
};

/// If the user declared an empty label, fall back to the code so that
/// label() never returns an empty string for a valid entry.
template <typename E>
[[nodiscard]] constexpr std::string_view label_or_code() noexcept {
    if constexpr (constexpr std::string_view l = E::label; l.empty()) {
        return std::string_view{E::code};
    } else {
        return l;
    }
}

template <typename E>
[[nodiscard]] constexpr TagValMetadata metadata_of() noexcept {
    return TagValMetadata{
        .code = std::string_view{E::code},
        .label = label_or_code<E>(),
        .icon = std::string_view{E::icon},
        .color = std::string_view{E::color},
    };
}

/// Pinned, per-entry TagValMetadata constant. Inline-variable templates are
/// ODR-merged across translation units, so every TU sees the same address for
/// `metadata_v<E>`. Registry stores pointers into this storage instead of
/// copies, which keeps handle identity stable even when extern entries register
/// after main() begins.
template <typename E>
inline constexpr TagValMetadata metadata_v = metadata_of<E>();

}  // namespace tagval
