/// Demonstrates the optional nlohmann/json integration: tag values serialize
/// to a bare code string and deserialize back via T::try_of().

#include <tagval/tagval.hpp>

#include <nlohmann/json.hpp>

#include <iostream>

class TransactionType : public tagval::OpenEnded<"tx_type", TransactionType> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(TransactionType, Debit, debit, "Debit")
    TAGVAL_ENTRY(TransactionType, Credit, credit, "Credit")

    using values_t = tagval::Values<Debit, Credit>;
};

int main() {
    const nlohmann::json j = TransactionType::debit();
    std::cout << "encoded: " << j.dump() << '\n';

    const auto recovered = j.get<TransactionType>();
    std::cout << "decoded: " << recovered << '\n';

    try {
        const nlohmann::json bad = "nonsense";
        (void)bad.get<TransactionType>();
    } catch (const tagval::UnknownCodeError& e) {
        std::cout << "rejected unknown: " << e.what() << '\n';
    }
    return 0;
}
