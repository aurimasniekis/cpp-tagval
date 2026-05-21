#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <format>

namespace {

class Direction : public tagval::ClosedEnded<"dir", Direction> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;
    TAGVAL_ENTRY(Direction, North, north)
    TAGVAL_ENTRY(Direction, South, south)
    using values_t = tagval::Values<North, South>;
};

TEST(Format, FormatsAsCode) {
    EXPECT_EQ(std::format("{}", Direction::north()), "north");
    EXPECT_EQ(std::format("[{}]", Direction::south()), "[south]");
}

TEST(Format, EmptyHandleFormatsAsEmpty) {
    Direction d;
    EXPECT_EQ(std::format("{}", d), "");
}

TEST(Format, NonDefaultSpecRejected) {
    Direction n = Direction::north();
    EXPECT_THROW((void)std::vformat("{:>10}", std::make_format_args(n)), std::format_error);
}

}  // namespace
