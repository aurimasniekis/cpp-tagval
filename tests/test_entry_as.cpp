#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

namespace {

class Doc : public tagval::ClosedEnded<"doc", Doc> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY_AS(Doc, Archived, is_archived, "archived", "Archived")
    TAGVAL_ENTRY_AS(Doc, Live, is_live, "live")

    using values_t = tagval::Values<Archived, Live>;
};

class Job : public tagval::OpenEnded<"job", Job> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    // Same explicit-code form works for OpenEnded via values_t.
    TAGVAL_ENTRY_AS(Job, FullTime, full_time, "full_time", "Full-time")

    using values_t = tagval::Values<FullTime>;
};

TEST(EntryAs, ClosedExplicitCodeDecouplesFromFunctionName) {
    EXPECT_EQ(Doc::is_archived().code(), "archived");
    EXPECT_EQ(Doc::is_archived().label(), "Archived");
    // Empty label still falls back to code.
    EXPECT_EQ(Doc::is_live().code(), "live");
    EXPECT_EQ(Doc::is_live().label(), "live");
    EXPECT_EQ(Doc::of("archived"), Doc::is_archived());
}

TEST(EntryAs, OpenValuesTExplicitCodeRoundTrips) {
    EXPECT_EQ(Job::full_time().code(), "full_time");
    EXPECT_EQ(Job::full_time().label(), "Full-time");
    EXPECT_EQ(Job::of("full_time"), Job::full_time());
}

}  // namespace
