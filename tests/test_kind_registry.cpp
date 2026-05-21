#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace kindreg_test {

class Status : public tagval::ClosedEnded<"kindreg.status", Status> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Status, Active, active, "Active")
    TAGVAL_ENTRY(Status, Inactive, inactive, "Inactive", "mdi:off")

    using values_t = tagval::Values<Active, Inactive>;
};

class DeviceKind : public tagval::OpenEnded<"kindreg.device_kind", DeviceKind> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(DeviceKind, Phone, phone, "Phone")
    TAGVAL_ENTRY(DeviceKind, Tablet, tablet, "Tablet")

    using values_t = tagval::Values<Phone, Tablet>;
};

class LateOpen : public tagval::OpenEnded<"kindreg.late_open", LateOpen> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(LateOpen, Builtin, builtin, "Built-in")

    using values_t = tagval::Values<Builtin>;
};

}  // namespace kindreg_test

TAGVAL_REGISTER_KIND(::kindreg_test::Status);
TAGVAL_REGISTER_KIND(::kindreg_test::DeviceKind);
TAGVAL_REGISTER_KIND(::kindreg_test::LateOpen);

// Extern entry registered into LateOpen after the kind is registered. The
// snapshot in test #6 must include it.
namespace kindreg_vendor {
TAGVAL_EXTERN_ENTRY(::kindreg_test::LateOpen, AfterTheFact, after_the_fact, "After The Fact");
}

namespace {

TEST(KindRegistry, RegisteredKindsAreDiscoverable) {
    const auto* status_kv = tagval::KindRegistry::find("kindreg.status");
    const auto* dev_kv = tagval::KindRegistry::find("kindreg.device_kind");
    const auto* late_kv = tagval::KindRegistry::find("kindreg.late_open");
    ASSERT_NE(status_kv, nullptr);
    ASSERT_NE(dev_kv, nullptr);
    ASSERT_NE(late_kv, nullptr);
    EXPECT_EQ(status_kv->kind_id(), "kindreg.status");
    EXPECT_EQ(dev_kv->kind_id(), "kindreg.device_kind");

    EXPECT_EQ(tagval::KindRegistry::find("kindreg.does_not_exist"), nullptr);
}

TEST(KindRegistry, CategoryMatchesBaseClass) {
    const auto* status_kv = tagval::KindRegistry::find("kindreg.status");
    const auto* dev_kv = tagval::KindRegistry::find("kindreg.device_kind");
    ASSERT_NE(status_kv, nullptr);
    ASSERT_NE(dev_kv, nullptr);

    EXPECT_EQ(status_kv->category(), tagval::KindCategory::Closed);
    EXPECT_TRUE(status_kv->is_closed_ended());
    EXPECT_FALSE(status_kv->is_open_ended());

    EXPECT_EQ(dev_kv->category(), tagval::KindCategory::Open);
    EXPECT_TRUE(dev_kv->is_open_ended());
    EXPECT_FALSE(dev_kv->is_closed_ended());
}

TEST(KindRegistry, FilteredViewsPartitionAll) {
    const auto all = tagval::KindRegistry::all();
    const auto closed_count = std::ranges::distance(tagval::KindRegistry::all_closed());
    const auto open_count = std::ranges::distance(tagval::KindRegistry::all_open());
    EXPECT_EQ(static_cast<std::size_t>(closed_count + open_count), all.size());

    // Every closed-ended view satisfies the predicate.
    for (const auto& kv : tagval::KindRegistry::all_closed()) {
        EXPECT_TRUE(kv.is_closed_ended());
        EXPECT_EQ(kv.category(), tagval::KindCategory::Closed);
    }
    for (const auto& kv : tagval::KindRegistry::all_open()) {
        EXPECT_TRUE(kv.is_open_ended());
        EXPECT_EQ(kv.category(), tagval::KindCategory::Open);
    }

    // Our specific kinds show up in the right bucket.
    auto contains_id = [](auto rng, std::string_view id) {
        return std::ranges::any_of(rng, [&](const auto& kv) { return kv.kind_id() == id; });
    };
    EXPECT_TRUE(contains_id(tagval::KindRegistry::all_closed(), "kindreg.status"));
    EXPECT_TRUE(contains_id(tagval::KindRegistry::all_open(), "kindreg.device_kind"));
    EXPECT_FALSE(contains_id(tagval::KindRegistry::all_open(), "kindreg.status"));
    EXPECT_FALSE(contains_id(tagval::KindRegistry::all_closed(), "kindreg.device_kind"));
}

TEST(KindRegistry, ValuesMatchAllValues) {
    const auto* status_kv = tagval::KindRegistry::find("kindreg.status");
    ASSERT_NE(status_kv, nullptr);

    const auto snapshot = status_kv->values();
    const auto direct = kindreg_test::Status::all_values();

    ASSERT_EQ(snapshot.size(), direct.size());
    for (std::size_t i = 0; i < snapshot.size(); ++i) {
        EXPECT_EQ(snapshot[i].code, direct[i].code);
        EXPECT_EQ(snapshot[i].label, direct[i].label);
        EXPECT_EQ(snapshot[i].icon, direct[i].icon);
        EXPECT_EQ(snapshot[i].color, direct[i].color);
    }
}

TEST(KindRegistry, ForEachMatchesValuesSnapshot) {
    const auto* dev_kv = tagval::KindRegistry::find("kindreg.device_kind");
    ASSERT_NE(dev_kv, nullptr);

    std::vector<std::string> walked;
    dev_kv->for_each([&](const tagval::KindEntryView& e) { walked.emplace_back(e.code); });

    std::vector<std::string> snapshot_codes;
    for (const auto& e : dev_kv->values()) {
        snapshot_codes.emplace_back(e.code);
    }

    EXPECT_EQ(walked, snapshot_codes);
}

TEST(KindRegistry, OpenEndedSnapshotIncludesLateExternEntries) {
    const auto* late_kv = tagval::KindRegistry::find("kindreg.late_open");
    ASSERT_NE(late_kv, nullptr);

    // The vendor extern entry registered at namespace scope above is part of
    // the kind's all_values() and must therefore appear in the registry's
    // snapshot — proving the snapshot is fresh, not captured at registration
    // time.
    const auto snapshot = late_kv->values();
    const bool has_late =
        std::ranges::any_of(snapshot, [](const auto& e) { return e.code == "after_the_fact"; });
    EXPECT_TRUE(has_late);

    const auto found = late_kv->find("after_the_fact");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->label, "After The Fact");

    EXPECT_FALSE(late_kv->find("missing").has_value());
}

TEST(KindRegistry, ReRegistrationIsIdempotent) {
    const auto before = tagval::KindRegistry::all().size();
    tagval::KindRegistry::register_kind<kindreg_test::Status>();
    tagval::KindRegistry::register_kind<kindreg_test::Status>();
    tagval::KindRegistry::register_kind<kindreg_test::DeviceKind>();
    const auto after = tagval::KindRegistry::all().size();
    EXPECT_EQ(before, after);
}

// Compile-time: the TagValKind concept rejects types that don't derive from
// ClosedEnded or OpenEnded, so register_kind<NonKind>() is ill-formed. We
// assert the concept directly rather than wrap the call in `requires { ... }`
// because the constraint-failure diagnostic is not consistently soft across
// compilers when applied to a constrained function call.
struct NotAKind {};
static_assert(!tagval::detail::TagValKind<int>);
static_assert(!tagval::detail::TagValKind<NotAKind>);
static_assert(tagval::detail::TagValKind<kindreg_test::Status>);
static_assert(tagval::detail::TagValKind<kindreg_test::DeviceKind>);
static_assert(tagval::detail::category_of<kindreg_test::Status>() == tagval::KindCategory::Closed);
static_assert(tagval::detail::category_of<kindreg_test::DeviceKind>() ==
              tagval::KindCategory::Open);

}  // namespace
