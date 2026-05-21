// Compile-pass test: SFINAE-style probe for the static_assert behind
// ClosedEnded::value<E>() — calling it with an entry not in values_t must be
// ill-formed. We can't observe `static_assert` failure with a `requires`
// expression directly (a failed assertion is a hard error, not SFINAE), but we
// can at least prove the well-formed call compiles, and document the intent.

#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

namespace {

class Mode : public tagval::ClosedEnded<"mode", Mode> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Mode, On, on)
    TAGVAL_ENTRY(Mode, Off, off)

    using values_t = tagval::Values<On, Off>;
};

// Stray entry whose owner is Mode but which is intentionally absent from
// values_t. Calling `Mode::value<Stray>()` would static_assert-fail; we keep
// the type around to assert it does *not* satisfy values_t::contains.
struct Stray : tagval::Entry<Mode, "stray"> {};

static_assert(Mode::values_t::contains<Mode::On>);
static_assert(Mode::values_t::contains<Mode::Off>);
static_assert(!Mode::values_t::contains<Stray>);

TEST(ClosedStaticAssert, ContainsReportsMembership) {
    EXPECT_EQ(Mode::on().code(), "on");
    EXPECT_EQ(Mode::off().code(), "off");
}

}  // namespace
