#pragma once

/// @file
/// @brief Public kind-level metadata struct.

#include <commons/color.hpp>
#include <commons/icon.hpp>

#include <optional>
#include <string_view>

namespace tagval {

/// Runtime metadata describing a tag-value kind (e.g. "device_kind").
///
/// Lifetime: every string_view here must refer to storage that outlives the
/// descriptor. For a typical usage where `make_descriptor()` returns string
/// literals, this is automatic.
struct TagValDescriptor {
    // NOLINTBEGIN(readability-redundant-member-init)
    std::string_view id;                  ///< Stable identifier (e.g. "device_kind"). Required.
    std::string_view name{};              ///< Human-readable label (e.g. "Device Kind").
    std::optional<comms::Icon> icon{};    ///< Optional UI icon.
    std::optional<comms::Color> color{};  ///< Optional UI accent.
    // NOLINTEND(readability-redundant-member-init)

    [[nodiscard]] friend constexpr bool operator==(const TagValDescriptor&,
                                                   const TagValDescriptor&) noexcept = default;
};

}  // namespace tagval
