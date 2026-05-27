#pragma once

/// @file
/// @brief CRTP HandleBase used by ClosedEnded and OpenEnded.
///
/// A handle's runtime data is a single `const TagValMetadata*`. The Derived
/// class itself *is* the handle. The marker class TagValBaseTag lets generic
/// specializations of std::hash, std::formatter and operator<< match every
/// tagval type via std::derived_from.

#include <tagval/descriptor.hpp>
#include <tagval/entry.hpp>
#include <tagval/error.hpp>

#include <commons/color.hpp>
#include <commons/fixed_string.hpp>
#include <commons/icon.hpp>

#include <compare>
#include <concepts>
#include <expected>
#include <optional>
#include <span>
#include <string_view>

namespace tagval::detail {

/// Empty marker base — used by std::hash / std::formatter / operator<<
/// specializations to recognize all tagval value types.
class TagValBaseTag {};

template <typename T>
concept HasMakeDescriptor = requires {
    { T::make_descriptor() } -> std::same_as<TagValDescriptor>;
};

template <comms::FixedString Id, typename Derived>
[[nodiscard]] constexpr TagValDescriptor compute_descriptor() noexcept {
    if constexpr (HasMakeDescriptor<Derived>) {
        return Derived::make_descriptor();
    } else {
        return TagValDescriptor{.id = Id.view()};
    }
}

/// CRTP handle. Both ClosedEnded and OpenEnded derive from this; the concrete
/// user type then derives from one of those.
template <comms::FixedString Id, typename Derived>
class HandleBase : public TagValBaseTag {
public:
    /// Default-constructed handle: empty(), bool() == false. Useful as a
    /// not-yet-set sentinel.
    constexpr HandleBase() noexcept = default;

    [[nodiscard]] std::string_view code() const noexcept {
        return meta_ != nullptr ? meta_->code : std::string_view{};
    }

    [[nodiscard]] std::string_view label() const noexcept {
        return meta_ != nullptr ? meta_->label : std::string_view{};
    }

    [[nodiscard]] std::optional<comms::Icon> icon() const noexcept {
        return meta_ != nullptr ? meta_->icon : std::nullopt;
    }

    [[nodiscard]] std::optional<comms::Color> color() const noexcept {
        return meta_ != nullptr ? meta_->color : std::nullopt;
    }

    [[nodiscard]] bool empty() const noexcept {
        return meta_ == nullptr;
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return meta_ != nullptr;
    }

    [[nodiscard]] static constexpr std::string_view kind_id() noexcept {
        return Id.view();
    }

    [[nodiscard]] static constexpr TagValDescriptor descriptor() noexcept {
        return compute_descriptor<Id, Derived>();
    }

    [[nodiscard]] friend constexpr bool operator==(const HandleBase& a,
                                                   const HandleBase& b) noexcept {
        return a.meta_ == b.meta_;
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(const HandleBase& a,
                                                                    const HandleBase& b) noexcept {
        // Null handles sort before populated ones; among populated handles
        // ordering is lexicographic on code().
        const bool ae = a.meta_ == nullptr;
        if (const bool be = b.meta_ == nullptr; ae || be) {
            return static_cast<int>(be) <=> static_cast<int>(ae);
        }
        return a.meta_->code.compare(b.meta_->code) <=> 0;
    }

protected:
    explicit constexpr HandleBase(const TagValMetadata* m) noexcept : meta_(m) {}

    const TagValMetadata* meta_ = nullptr;

    /// Build a Derived handle from a stable metadata pointer.
    [[nodiscard]] static Derived make_handle(const TagValMetadata* m) noexcept {
        Derived d;
        d.HandleBase::meta_ = m;
        return d;
    }

    /// Linear search a metadata range by code.
    [[nodiscard]] static const TagValMetadata* find_in(const std::span<const TagValMetadata> values,
                                                       const std::string_view code) noexcept {
        for (const auto& m : values) {
            if (m.code == code) {
                return &m;
            }
        }
        return nullptr;
    }

    [[nodiscard]] static Derived of_impl(const std::span<const TagValMetadata> values,
                                         const std::string_view code) {
        const TagValMetadata* m = find_in(values, code);
        if (m == nullptr) {
            std::string msg = "tagval: unknown code '";
            msg.append(code);
            msg += "' for kind '";
            msg.append(Id.view());
            msg += "'";
            throw UnknownCodeError{msg};
        }
        return make_handle(m);
    }

    [[nodiscard]] static std::expected<Derived, ParseError>
    try_of_impl(const std::span<const TagValMetadata> values,
                const std::string_view code) noexcept {
        const TagValMetadata* m = find_in(values, code);
        if (m == nullptr) {
            return std::unexpected(ParseError{.code = std::string{code}, .kind_id = Id.view()});
        }
        return make_handle(m);
    }
};

}  // namespace tagval::detail
