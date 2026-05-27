/// Demonstrates kind-level (TagValDescriptor) and per-entry (icon/color via
/// trailing TAGVAL_ENTRY arguments) metadata.

#include <tagval/tagval.hpp>

#include <commons/literals.hpp>

#include <iostream>
#include <string>
#include <string_view>

using namespace comms::literals;

class Severity : public tagval::ClosedEnded<"severity", Severity> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    [[maybe_unused]] static constexpr tagval::TagValDescriptor make_descriptor() noexcept {
        return tagval::TagValDescriptor{
            .id = "severity",
            .name = "Alert Severity",
            .icon = "mdi:alert"_icon,
            .color = "#aa0000"_color,
        };
    }

    TAGVAL_ENTRY(Severity, Info, info, "Info", "mdi:information", "#3366cc")
    TAGVAL_ENTRY(Severity, Warn, warn, "Warn", "mdi:alert", "#cc9900")
    TAGVAL_ENTRY(Severity, Error, error, "Error", "mdi:alert-circle", "#cc0000")

    using values_t = tagval::Values<Info, Warn, Error>;
};

int main() {
    constexpr auto k = Severity::descriptor();
    std::cout << "Kind: " << k.id << " — " << k.name
              << " (icon=" << (k.icon ? k.icon->value() : std::string_view{"-"}) << ")\n";

    std::cout << "Values:\n";
    for (const auto& [code, label, icon, color] : Severity::all_values()) {
        std::cout << "  [" << (icon ? icon->value() : std::string_view{"-"}) << "] " << code << " ("
                  << label << ") color=" << (color ? color->to_hex_string() : std::string{"-"})
                  << '\n';
    }
    return 0;
}
