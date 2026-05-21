#include <tagval/tagval.hpp>

#include <gtest/gtest.h>

#include <parcel/parcel.h>

namespace {

class DeviceKind : public tagval::OpenEnded<"device_kind", DeviceKind> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(DeviceKind, Phone, phone, "Phone")
    TAGVAL_ENTRY(DeviceKind, Tablet, tablet, "Tablet")

    using values_t = tagval::Values<Phone, Tablet>;
};

TEST(Parcel, RoundTripPreservesValue) {
    using Cell = tagval::TagValCell<DeviceKind>;

    parcel::ParcelRegistry reg;
    reg.register_cells<Cell>();

    const Cell cell{DeviceKind::phone()};
    const auto j = cell.to_json();

    const auto restored = reg.cell_from_json(j);
    auto* round_tripped = dynamic_cast<Cell*>(restored.get());
    ASSERT_NE(round_tripped, nullptr);
    EXPECT_EQ(round_tripped->value, DeviceKind::phone());
}

}  // namespace
