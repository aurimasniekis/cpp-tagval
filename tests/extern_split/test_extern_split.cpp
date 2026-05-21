#include <gtest/gtest.h>

#include "kind.hpp"

#include <algorithm>
#include <string_view>
#include <vector>

// Register Kind with the program-wide KindRegistry. Placed in the consumer
// TU so the registrar can never be dead-stripped — the question under test is
// whether vendor extern entries (in separate TUs) still surface through the
// resulting KindView.
TAGVAL_REGISTER_KIND(Kind);

namespace {

// The vendor TUs are linked into this exe as an OBJECT-library (their .o
// files appear directly on the link line). Nothing in this TU references the
// inline `_registered_` variables that TAGVAL_EXTERN_ENTRY[_AS] declares, so
// the lookups below succeed *only* because [[gnu::used]] keeps the registrar
// symbols from being dead-stripped by the linker.

TEST(ExternSplit, ExternEntriesSurviveStaticArchive) {
    EXPECT_EQ(Kind::of("smart_watch").label(), "Smart Watch");
    EXPECT_EQ(Kind::of("fridge_cam").label(), "Fridge Cam");
}

TEST(ExternSplit, BuiltinAndExternSharePointerIdentity) {
    EXPECT_EQ(Kind::of("smart_watch"), Kind::of("smart_watch"));
    EXPECT_EQ(Kind::of("builtin"), Kind::builtin());
}

TEST(ExternSplit, AllValuesContainsBuiltinAndBothVendors) {
    auto values = Kind::all_values();
    auto has = [&](std::string_view code) {
        return std::ranges::any_of(values, [&](const auto& m) { return m.code == code; });
    };
    EXPECT_TRUE(has("builtin"));
    EXPECT_TRUE(has("smart_watch"));
    EXPECT_TRUE(has("fridge_cam"));
}

// The above tests probe the per-kind OpenEndedRegistry. The next three probe
// the program-wide KindRegistry — same question, but asked through the
// type-erased KindView. This is the real plugin case: vendor TUs contribute
// entries from a separate object library, the consumer registers the kind
// once, and a downstream tool (docs generator, OpenAPI emitter, …) iterates
// KindRegistry and expects to see *all* entries, vendor-contributed ones
// included.

TEST(ExternSplit, KindRegistrySeesTheKind) {
    const auto* kv = tagval::KindRegistry::find("split_kind");
    ASSERT_NE(kv, nullptr);
    EXPECT_EQ(kv->kind_id(), "split_kind");
    EXPECT_TRUE(kv->is_open_ended());
}

TEST(ExternSplit, KindViewValuesIncludeVendorExternEntries) {
    const auto* kv = tagval::KindRegistry::find("split_kind");
    ASSERT_NE(kv, nullptr);

    const auto snapshot = kv->values();
    auto has = [&](std::string_view code) {
        return std::ranges::any_of(snapshot, [&](const auto& e) { return e.code == code; });
    };
    EXPECT_TRUE(has("builtin"));
    EXPECT_TRUE(has("smart_watch"));  // contributed by vendor_a.cpp
    EXPECT_TRUE(has("fridge_cam"));   // contributed by vendor_b.cpp

    // Vendor labels also flow through: confirms the snapshot reads the live
    // metadata for each extern entry, not a captured stub.
    const auto sw = kv->find("smart_watch");
    ASSERT_TRUE(sw.has_value());
    EXPECT_EQ(sw->label, "Smart Watch");

    const auto fc = kv->find("fridge_cam");
    ASSERT_TRUE(fc.has_value());
    EXPECT_EQ(fc->label, "Fridge Cam");
}

TEST(ExternSplit, KindViewForEachVisitsVendorEntries) {
    const auto* kv = tagval::KindRegistry::find("split_kind");
    ASSERT_NE(kv, nullptr);

    std::vector<std::string_view> codes;
    kv->for_each([&](const tagval::KindEntryView& e) { codes.push_back(e.code); });

    auto has = [&](const std::string_view code) {
        return std::ranges::find(codes, code) != codes.end();
    };
    EXPECT_TRUE(has("builtin"));
    EXPECT_TRUE(has("smart_watch"));
    EXPECT_TRUE(has("fridge_cam"));
}

}  // namespace
