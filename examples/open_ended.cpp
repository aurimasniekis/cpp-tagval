/// Demonstrates an open-ended tag-value type whose predefined entries are
/// listed in `values_t` (the same shape as ClosedEnded). External entries
/// can extend the kind via TAGVAL_EXTERN_ENTRY (see extern_entries.cpp).

#include <tagval/tagval.hpp>

#include <iostream>

class DeviceKind : public tagval::OpenEnded<"device_kind", DeviceKind> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    [[maybe_unused]] static constexpr tagval::TagValDescriptor make_descriptor() noexcept {
        return tagval::TagValDescriptor{.id = "device_kind", .name = "Device Kind"};
    }

    TAGVAL_ENTRY(DeviceKind, Phone, phone, "Phone")
    TAGVAL_ENTRY(DeviceKind, Tablet, tablet, "Tablet")
    TAGVAL_ENTRY(DeviceKind, Laptop, laptop, "Laptop")

    using values_t = tagval::Values<Phone, Tablet, Laptop>;
};

int main() {
    std::cout << "All " << DeviceKind::descriptor().name << " values:\n";
    for (const auto& m : DeviceKind::all_values()) {
        std::cout << "  - " << m.code << " (" << m.label << ")\n";
    }

    const auto found = DeviceKind::of("phone");
    std::cout << "of('phone') == phone(): " << std::boolalpha << (found == DeviceKind::phone())
              << '\n';
    return 0;
}
