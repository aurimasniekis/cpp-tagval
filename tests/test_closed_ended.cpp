#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

class Status : public tagval::ClosedEnded<"status", Status> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Status, Active, active)
    TAGVAL_ENTRY(Status, Inactive, inactive, "Inactive", "mdi:off")
    TAGVAL_ENTRY(Status, Pending, pending, "Pending")

    using values_t = tagval::Values<Active, Inactive, Pending>;
};

TEST(ClosedEnded, ValueAccessorsExposeMetadata) {
    EXPECT_EQ(Status::active().code(), "active");
    // Empty Label falls back to code.
    EXPECT_EQ(Status::active().label(), "active");
    EXPECT_EQ(Status::inactive().label(), "Inactive");
    ASSERT_TRUE(Status::inactive().icon());
    EXPECT_EQ(Status::inactive().icon()->value(), "mdi:off");
    EXPECT_EQ(Status::pending().label(), "Pending");
}

TEST(ClosedEnded, ValueByEntryTypeMatchesAccessor) {
    EXPECT_EQ(&Status::value<Status::Active>(), &Status::value<Status::Active>());
    EXPECT_EQ(Status::value<Status::Active>(), Status::active());
}

TEST(ClosedEnded, OfReturnsHandleAndThrowsOnMiss) {
    EXPECT_EQ(Status::of("active"), Status::active());
    EXPECT_EQ(Status::of("inactive"), Status::inactive());
    EXPECT_THROW((void)Status::of("none"), tagval::UnknownCodeError);
}

TEST(ClosedEnded, TryOfMissReportsCodeAndKindId) {
    auto exp = Status::try_of("none");
    ASSERT_FALSE(exp.has_value());
    EXPECT_EQ(exp.error().code, "none");
    EXPECT_EQ(exp.error().kind_id, "status");

    auto ok = Status::try_of("pending");
    ASSERT_TRUE(ok.has_value());
    EXPECT_EQ(*ok, Status::pending());
}

TEST(ClosedEnded, AllValuesMatchesValuesTSize) {
    const auto values = Status::all_values();
    EXPECT_EQ(values.size(), Status::values_t::size);
    EXPECT_EQ(values.size(), 3u);

    const bool has_active = std::ranges::any_of(
        values, [](const auto& m) { return m.code == std::string_view{"active"}; });
    EXPECT_TRUE(has_active);
}

TEST(ClosedEnded, KindIdAndDescriptorAvailableWithoutMakeDescriptor) {
    EXPECT_EQ(Status::kind_id(), "status");
    EXPECT_EQ(Status::descriptor().id, "status");
}

}  // namespace
