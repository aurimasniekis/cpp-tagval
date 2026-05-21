#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

namespace {

class Plain : public tagval::ClosedEnded<"plain", Plain> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Plain, A, a)

    using values_t = tagval::Values<A>;
};

class Decorated : public tagval::ClosedEnded<"decorated", Decorated> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Decorated, A, a)

    using values_t = tagval::Values<A>;

    [[maybe_unused]] static constexpr tagval::TagValDescriptor make_descriptor() noexcept {
        return tagval::TagValDescriptor{
            .id = "decorated",
            .name = "Decorated Kind",
            .icon = "mdi:star",
            .color = "#ff8800",
        };
    }
};

TEST(Descriptor, DefaultsWhenMakeDescriptorAbsent) {
    const auto [id, name, icon, color] = Plain::descriptor();
    EXPECT_EQ(id, "plain");
    EXPECT_EQ(name, "");
    EXPECT_EQ(icon, "");
    EXPECT_EQ(color, "");
}

TEST(Descriptor, MakeDescriptorOverridesAllFields) {
    const auto [id, name, icon, color] = Decorated::descriptor();
    EXPECT_EQ(id, "decorated");
    EXPECT_EQ(name, "Decorated Kind");
    EXPECT_EQ(icon, "mdi:star");
    EXPECT_EQ(color, "#ff8800");
}

}  // namespace
