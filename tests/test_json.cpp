#include <tagval/tagval.hpp>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

namespace {

class Color : public tagval::ClosedEnded<"color", Color> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;
    TAGVAL_ENTRY(Color, Red, red, "Red")
    TAGVAL_ENTRY(Color, Green, green, "Green")
    using values_t = tagval::Values<Red, Green>;
};

TEST(Json, ToJsonEmitsBareCode) {
    const nlohmann::json j = Color::red();
    EXPECT_EQ(j.get<std::string>(), "red");
}

TEST(Json, FromJsonRoundTrips) {
    const nlohmann::json j = "green";
    const auto c = j.get<Color>();
    EXPECT_EQ(c, Color::green());
}

TEST(Json, UnknownCodeThrowsUnknownCodeError) {
    nlohmann::json j = "purple";
    EXPECT_THROW((void)j.get<Color>(), tagval::UnknownCodeError);
}

}  // namespace
