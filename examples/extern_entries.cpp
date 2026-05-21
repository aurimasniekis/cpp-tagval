/// Demonstrates plugin-style entry declarations: predefined entries live in
/// the kind's values_t, while vendor namespaces add new values via
/// TAGVAL_EXTERN_ENTRY without modifying the kind class.

#include <tagval/tagval.hpp>

#include <iostream>

class Plugin : public tagval::OpenEnded<"plugin", Plugin> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Plugin, Builtin, builtin, "Built-in")

    using values_t = tagval::Values<Builtin>;
};

namespace vendor_a {
TAGVAL_EXTERN_ENTRY(::Plugin, SmartWatch, smart_watch, "Smart Watch");
}

namespace vendor_b {
TAGVAL_EXTERN_ENTRY_AS(::Plugin, FridgeCam, fridge_cam, "fridge_cam", "Fridge Cam");
}

int main() {
    std::cout << "All Plugin values (built-in + plugins):\n";
    for (const auto& m : Plugin::all_values()) {
        std::cout << "  - " << m.code << " (" << m.label << ")\n";
    }

    std::cout << "vendor_a::smart_watch().code() = " << vendor_a::smart_watch().code() << '\n';
    std::cout << "Plugin::of('fridge_cam') == vendor_b::fridge_cam(): " << std::boolalpha
              << (Plugin::of("fridge_cam") == vendor_b::fridge_cam()) << '\n';
    return 0;
}
