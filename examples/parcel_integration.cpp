/// Demonstrates the optional cpp-parcel integration: wrap a tag-value handle
/// in a TagValCell and round-trip it through ParcelRegistry::cell_from_json.

#include <tagval/tagval.hpp>

#include <iostream>

#include <parcel/parcel.h>

class PaymentMethod : public tagval::OpenEnded<"payment_method", PaymentMethod> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(PaymentMethod, Card, card, "Card")
    TAGVAL_ENTRY(PaymentMethod, BankTransfer, bank, "Bank")

    using values_t = tagval::Values<Card, BankTransfer>;
};

int main() {
    using Cell = tagval::TagValCell<PaymentMethod>;

    const Cell cell{PaymentMethod::card()};
    const auto j = cell.to_json();
    std::cout << "encoded: " << j.dump() << '\n';

    ::parcel::ParcelRegistry reg;
    reg.register_cells<Cell>();
    const auto decoded = reg.cell_from_json(j);
    if (const auto* typed = dynamic_cast<Cell*>(decoded.get()); typed != nullptr) {
        std::cout << "decoded: " << typed->value << '\n';
    }
    return 0;
}
