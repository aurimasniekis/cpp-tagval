#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

// Predefined entries declared with TAGVAL_ENTRY (no registrar) and listed in
// values_t — same shape as ClosedEnded.
class Kind : public tagval::OpenEnded<"kind", Kind> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Kind, Phone, phone, "Phone")
    TAGVAL_ENTRY(Kind, Tablet, tablet, "Tablet", "mdi:tablet")

    using values_t = tagval::Values<Phone, Tablet>;
};

TEST(OpenEnded, ValuesTAccessorsExposeMetadata) {
    EXPECT_EQ(Kind::phone().code(), "phone");
    EXPECT_EQ(Kind::phone().label(), "Phone");
    EXPECT_EQ(Kind::tablet().icon(), "mdi:tablet");
}

TEST(OpenEnded, AllValuesIncludesValuesTEntries) {
    const auto values = Kind::all_values();
    EXPECT_EQ(values.size(), Kind::values_t::size);

    const bool has_phone = std::ranges::any_of(
        values, [](const auto& m) { return m.code == std::string_view{"phone"}; });
    const bool has_tablet = std::ranges::any_of(
        values, [](const auto& m) { return m.code == std::string_view{"tablet"}; });
    EXPECT_TRUE(has_phone);
    EXPECT_TRUE(has_tablet);
}

TEST(OpenEnded, OfRoundTripsValuesTEntry) {
    EXPECT_EQ(Kind::of("phone"), Kind::phone());
    EXPECT_EQ(Kind::of("tablet"), Kind::tablet());
    EXPECT_THROW((void)Kind::of("absent"), tagval::UnknownCodeError);
}

TEST(OpenEnded, TryOfMissReportsCodeAndKindId) {
    auto exp = Kind::try_of("absent");
    ASSERT_FALSE(exp.has_value());
    EXPECT_EQ(exp.error().code, "absent");
    EXPECT_EQ(exp.error().kind_id, "kind");
}

TEST(OpenEnded, ValueByEntryTypeMatchesAccessor) {
    EXPECT_EQ(Kind::value<Kind::Phone>(), Kind::phone());
}

}  // namespace
