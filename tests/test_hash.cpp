#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <functional>
#include <unordered_set>

namespace {

class Mood : public tagval::ClosedEnded<"mood", Mood> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;
    TAGVAL_ENTRY(Mood, Happy, happy)
    TAGVAL_ENTRY(Mood, Sad, sad)
    using values_t = tagval::Values<Happy, Sad>;
};

class Tone : public tagval::ClosedEnded<"tone", Tone> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;
    // Same code as Mood::Happy — hash should not collide.
    TAGVAL_ENTRY(Tone, Happy, happy)
    using values_t = tagval::Values<Happy>;
};

TEST(Hash, EqualValuesShareHash) {
    constexpr std::hash<Mood> h;
    EXPECT_EQ(h(Mood::happy()), h(Mood::happy()));
    EXPECT_NE(h(Mood::happy()), h(Mood::sad()));
}

TEST(Hash, KindIdDistinguishesAcrossKinds) {
    constexpr std::hash<Mood> hm;
    constexpr std::hash<Tone> ht;
    EXPECT_NE(hm(Mood::happy()), ht(Tone::happy()));
}

TEST(Hash, UsableInUnorderedContainer) {
    std::unordered_set<Mood> set;
    set.insert(Mood::happy());
    set.insert(Mood::sad());
    set.insert(Mood::happy());
    EXPECT_EQ(set.size(), 2u);
    EXPECT_TRUE(set.contains(Mood::happy()));
}

}  // namespace
