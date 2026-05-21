/// Demonstrates std::format and operator<< for tag-value types — both render
/// the bare code() so values drop into log lines transparently.

#include <tagval/tagval.hpp>

#include <format>
#include <iostream>

class Direction : public tagval::ClosedEnded<"direction", Direction> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Direction, North, north, "North")
    TAGVAL_ENTRY(Direction, South, south, "South")
    TAGVAL_ENTRY(Direction, East, east, "East")
    TAGVAL_ENTRY(Direction, West, west, "West")

    using values_t = tagval::Values<North, South, East, West>;
};

int main() {
    std::cout << std::format("Heading: {}\n", Direction::north());
    std::cout << "ostream: " << Direction::east() << '\n';
    std::cout << std::format("All 4: {}, {}, {}, {}\n",
                             Direction::north(),
                             Direction::south(),
                             Direction::east(),
                             Direction::west());
    return 0;
}
