#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <concepts>

namespace {

class KindA : public tagval::ClosedEnded<"a", KindA> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;
    TAGVAL_ENTRY(KindA, X, x)
    using values_t = tagval::Values<X>;
};

class KindB : public tagval::ClosedEnded<"b", KindB> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;
    TAGVAL_ENTRY(KindB, X, x)
    using values_t = tagval::Values<X>;
};

static_assert(!std::equality_comparable_with<KindA, KindB>);
static_assert(!std::convertible_to<KindA, KindB>);
static_assert(!std::convertible_to<KindB, KindA>);

TEST(CrossKind, KindIdsDistinguishOwners) {
    EXPECT_NE(KindA::kind_id(), KindB::kind_id());
    // Both share the literal code "x" but the entry types are distinct types.
    EXPECT_EQ(KindA::x().code(), "x");
    EXPECT_EQ(KindB::x().code(), "x");
}

}  // namespace
