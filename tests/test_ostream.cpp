#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <sstream>

namespace {

class Tag : public tagval::ClosedEnded<"tag", Tag> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;
    TAGVAL_ENTRY(Tag, Red, red)
    TAGVAL_ENTRY(Tag, Blue, blue)
    using values_t = tagval::Values<Red, Blue>;
};

TEST(Ostream, WritesCode) {
    std::ostringstream os;
    os << Tag::red();
    EXPECT_EQ(os.str(), "red");
}

TEST(Ostream, EmptyHandleWritesEmpty) {
    std::ostringstream os;
    os << Tag{};
    EXPECT_EQ(os.str(), "");
}

}  // namespace
