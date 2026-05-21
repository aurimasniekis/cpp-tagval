#pragma once

/// @file
/// @brief Per-Owner runtime registry used by OpenEnded kinds.
///
/// External entries register themselves at static-init time via the
/// TAGVAL_EXTERN_ENTRY / TAGVAL_EXTERN_ENTRY_AS macros. Each entry contributes
/// a pointer to its pinned `metadata_v<E>` constant; the registry stores
/// `const TagValMetadata*` so vector reallocations never invalidate handle
/// identity.
///
/// This is the per-OpenEnded-kind extern-entry storage. For the program-wide
/// index of every registered kind, see `<tagval/kind_registry.hpp>`.

#include <tagval/entry.hpp>

#include <algorithm>
#include <span>
#include <string_view>
#include <vector>

namespace tagval {

/// Runtime list of metadata pointers for a single OpenEnded kind. The Owner
/// template parameter keeps registries for distinct kinds isolated.
template <typename Owner>
class OpenEndedRegistry {
public:
    /// Register a pinned metadata pointer. Idempotent on `->code`: a second
    /// call with a pointer whose code is already known is a no-op.
    static void add(const TagValMetadata* m) {
        if (m == nullptr) {
            return;
        }
        auto& v = storage();
        if (std::ranges::any_of(v, [&](const auto* x) { return x->code == m->code; })) {
            return;
        }
        v.push_back(m);
    }

    [[nodiscard]] static std::span<const TagValMetadata* const> all() noexcept {
        return std::span<const TagValMetadata* const>{storage()};
    }

    [[nodiscard]] static const TagValMetadata* find(std::string_view code) noexcept {
        const auto& v = storage();
        const auto it = std::ranges::find_if(v, [&](const auto* m) { return m->code == code; });
        return it == v.end() ? nullptr : *it;
    }

private:
    [[nodiscard]] static std::vector<const TagValMetadata*>& storage() {
        static std::vector<const TagValMetadata*> v;
        return v;
    }
};

}  // namespace tagval
