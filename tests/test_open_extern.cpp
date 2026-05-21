#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

// Predefined entries via TAGVAL_ENTRY + values_t; plugin entries via
// TAGVAL_EXTERN_ENTRY register at namespace scope.
class Plugin : public tagval::OpenEnded<"plugin", Plugin> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Plugin, Builtin, builtin, "Built-in")

    using values_t = tagval::Values<Builtin>;
};

}  // namespace

namespace vendor_a {
TAGVAL_EXTERN_ENTRY(::Plugin, SmartWatch, smart_watch, "Smart Watch");
}

namespace vendor_b {
TAGVAL_EXTERN_ENTRY_AS(::Plugin, FridgeCam, fridge_cam, "fridge_cam", "Fridge Cam");
}

namespace {

TEST(OpenExtern, ExternEntryAccessorReturnsCanonicalHandle) {
    EXPECT_EQ(vendor_a::smart_watch().code(), "smart_watch");
    EXPECT_EQ(vendor_a::smart_watch().label(), "Smart Watch");
    EXPECT_EQ(vendor_b::fridge_cam().code(), "fridge_cam");
}

TEST(OpenExtern, ExternEntryVisibleViaOwnerOf) {
    EXPECT_EQ(Plugin::of("smart_watch"), vendor_a::smart_watch());
    EXPECT_EQ(Plugin::of("fridge_cam"), vendor_b::fridge_cam());
}

TEST(OpenExtern, AllValuesIncludesValuesTAndExternEntries) {
    const auto values = Plugin::all_values();
    auto has = [&](std::string_view code) {
        return std::ranges::any_of(values, [&](const auto& m) { return m.code == code; });
    };
    EXPECT_TRUE(has("builtin"));
    EXPECT_TRUE(has("smart_watch"));
    EXPECT_TRUE(has("fridge_cam"));
}

TEST(OpenExtern, ValuesTBuiltinAndExternComparePointerEqual) {
    // Both lookups should resolve to the same address inside the merged storage.
    EXPECT_EQ(Plugin::of("builtin"), Plugin::builtin());
    EXPECT_EQ(Plugin::of("smart_watch"), vendor_a::smart_watch());
}

}  // namespace
