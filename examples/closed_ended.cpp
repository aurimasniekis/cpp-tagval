/// Demonstrates a closed-ended tag-value type.
///
/// `Status` declares a fixed compile-time set of allowed values via
/// `tagval::Values<...>` and rejects unknown codes at of() / from_json(). The
/// in-class TAGVAL_ENTRY macro generates an Entry type plus a static
/// accessor; TAGVAL_ENTRY_AS lets the function name diverge from the code.

#include <tagval/tagval.hpp>

#include <iostream>

class Status : public tagval::ClosedEnded<"status", Status> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    [[maybe_unused]] static constexpr tagval::TagValDescriptor make_descriptor() noexcept {
        return tagval::TagValDescriptor{.id = "status", .name = "Status"};
    }

    TAGVAL_ENTRY(Status, Active, active)
    TAGVAL_ENTRY(Status, Inactive, inactive, "Inactive")
    TAGVAL_ENTRY_AS(Status, Archived, is_archived, "archived", "Archived")

    using values_t = tagval::Values<Active, Inactive, Archived>;
};

int main() {
    std::cout << "kind id: " << Status::kind_id() << '\n';
    std::cout << "values:\n";
    for (const auto& m : Status::all_values()) {
        std::cout << "  - " << m.code << " (" << m.label << ")\n";
    }

    const auto parsed = Status::of("active");
    std::cout << "of('active'): " << parsed << '\n';
    std::cout << "is_archived(): " << Status::is_archived().code() << '\n';

    try {
        (void)Status::of("nonsense");
    } catch (const tagval::UnknownCodeError& e) {
        std::cout << "rejected unknown: " << e.what() << '\n';
    }
    return 0;
}
