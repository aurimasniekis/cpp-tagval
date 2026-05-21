/// Demonstrates kind-level (TagValDescriptor) and per-entry (icon/color via
/// trailing TAGVAL_ENTRY arguments) metadata.

#include <tagval/tagval.hpp>

#include <iostream>

class Severity : public tagval::ClosedEnded<"severity", Severity> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    [[maybe_unused]] static constexpr tagval::TagValDescriptor make_descriptor() noexcept {
        return tagval::TagValDescriptor{
            .id = "severity",
            .name = "Alert Severity",
            .icon = "alert",
            .color = "#aa0000",
        };
    }

    TAGVAL_ENTRY(Severity, Info, info, "Info", "info", "#3366cc")
    TAGVAL_ENTRY(Severity, Warn, warn, "Warn", "warning", "#cc9900")
    TAGVAL_ENTRY(Severity, Error, error, "Error", "error", "#cc0000")

    using values_t = tagval::Values<Info, Warn, Error>;
};

int main() {
    constexpr auto k = Severity::descriptor();
    std::cout << "Kind: " << k.id << " — " << k.name << " (icon=" << k.icon << ")\n";

    std::cout << "Values:\n";
    for (const auto& [code, label, icon, color] : Severity::all_values()) {
        std::cout << "  [" << icon << "] " << code << " (" << label << ") color=" << color << '\n';
    }
    return 0;
}
