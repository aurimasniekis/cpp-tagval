#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <type_traits>

namespace {

class Letter : public tagval::ClosedEnded<"letter", Letter> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Letter, A, a)
    TAGVAL_ENTRY(Letter, B, b)
    TAGVAL_ENTRY(Letter, C, c)

    using values_t = tagval::Values<A, B, C>;
};

TEST(HandleBasics, DefaultConstructedHandleIsEmpty) {
    constexpr Letter l;
    EXPECT_TRUE(l.empty());
    EXPECT_FALSE(static_cast<bool>(l));
    EXPECT_EQ(l.code(), "");
    EXPECT_EQ(l.label(), "");
}

TEST(HandleBasics, EqualityIsByEntryIdentity) {
    EXPECT_EQ(Letter::a(), Letter::a());
    EXPECT_NE(Letter::a(), Letter::b());

    constexpr Letter empty1;
    constexpr Letter empty2;
    EXPECT_EQ(empty1, empty2);
    EXPECT_NE(empty1, Letter::a());
}

TEST(HandleBasics, OrderingIsLexicographicOnCode) {
    EXPECT_LT(Letter::a(), Letter::b());
    EXPECT_LT(Letter::b(), Letter::c());
    EXPECT_GT(Letter::c(), Letter::a());

    // Empty handle sorts before populated ones.
    constexpr Letter empty;
    EXPECT_LT(empty, Letter::a());
}

TEST(HandleBasics, CopyIsTriviallyCopyable) {
    static_assert(std::is_trivially_copyable_v<Letter>);
    const Letter src = Letter::a();
    const Letter dst = src;
    EXPECT_EQ(src, dst);
    EXPECT_EQ(dst.code(), "a");
}

}  // namespace
