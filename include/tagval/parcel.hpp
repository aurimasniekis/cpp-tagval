#pragma once

/// @file
/// @brief Optional cpp-parcel integration. Activates when parcel is on the
///        include path (or when TAGVAL_WITH_PARCEL=1 is defined by CMake).
///
/// Wire envelope (parcel framing of tagval's bare-string JSON adapter):
///
///     DeviceKind::Phone   <->   {"k":"tagval","v":"phone"}
///
/// Limitation: every TagValCell<T> instantiation shares kind_id="tagval", so a
/// single ParcelRegistry cannot dispatch by kind to multiple TagT. This matches
/// dimval's documented constraint and is fine for the typical site-knows-the-
/// type usage. Inner JSON validation: T::try_of() (called via the bare-string
/// from_json adapter) reports unknown codes, so cross-kind mismatches still
/// surface as exceptions when the codes don't happen to coincide.

#if !defined(TAGVAL_WITH_PARCEL)
#if __has_include(<parcel/parcel.h>)
#define TAGVAL_WITH_PARCEL 1
#else
#define TAGVAL_WITH_PARCEL 0
#endif
#endif

#if TAGVAL_WITH_PARCEL

#include <tagval/base.hpp>
#include <tagval/json_nlohmann.hpp>

#include <concepts>
#include <memory>
#include <string>
#include <string_view>

#include <parcel/parcel.h>

namespace tagval {

template <typename TagT>
    requires std::derived_from<TagT, detail::TagValBaseTag>
class TagValCell : public ::parcel::BaseCell<TagValCell<TagT>, TagT> {
    using base_t = ::parcel::BaseCell<TagValCell<TagT>, TagT>;

public:
    using base_t::base_t;
    using base_t::operator=;

    static constexpr std::string_view kind_id = "tagval";

    [[nodiscard]] std::string to_string() const override {
        return std::string{this->value.code()};
    }

    static ::parcel::cell_t from_json(::parcel::json_t const& j, ::parcel::ParcelRegistry const&) {
        auto v = base_t::template cell_from_json<TagT>(j, kind_id);
        auto cell = std::make_shared<TagValCell>(v);
        base_t::absorb_meta(j, cell);
        return cell;
    }

    static ::parcel::cell_type_descriptor_t descriptor() {
        static const auto d = std::make_shared<::parcel::SimpleCellTypeDescriptor<TagValCell>>(
            ::parcel::descriptor::MetaInfo{.name = "tagval::TagVal"});
        return d;
    }
};

}  // namespace tagval

#endif  // TAGVAL_WITH_PARCEL
