#pragma once

/// @file
/// @brief Program-wide registry of every tag-value kind that opts in via the
///        TAGVAL_REGISTER_KIND macro.
///
/// This is distinct from `OpenEndedRegistry<Owner>` in
/// `<tagval/openended_registry.hpp>`, which is per-OpenEnded-kind storage of
/// extern-entry pointers. KindRegistry indexes *kinds* — both ClosedEnded and
/// OpenEnded — and is intended for tools that want to enumerate the full set
/// of tag-value kinds in a program (API documentation, OpenAPI emitters,
/// debug/admin endpoints, etc.).
///
/// Registration is opt-in via the TAGVAL_REGISTER_KIND macro (declared in
/// `<tagval/macros.hpp>`); a kind that is never registered does not appear in
/// the registry even if it is otherwise used in the program.
///
/// Concurrency model mirrors OpenEndedRegistry: mutation only happens during
/// static-init (single-threaded by contract); reads after `main()` are safe
/// for concurrent readers.

#include <tagval/closed_ended.hpp>
#include <tagval/descriptor.hpp>
#include <tagval/entry.hpp>
#include <tagval/open_ended.hpp>

#include <commons/color.hpp>
#include <commons/fixed_string.hpp>
#include <commons/icon.hpp>

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace tagval {

namespace detail {
template <typename T>
class KindViewFactory;
}  // namespace detail

/// Which CRTP base a kind derives from.
enum class KindCategory { Closed, Open };

/// Flattened, copy-friendly view of a single entry. The string_views point
/// into the originating entry's NTTP storage and remain valid for the
/// program's lifetime, so a KindEntryView outlives any container it was
/// emitted into.
struct KindEntryView {
    std::string_view code;
    std::string_view label;
    std::optional<comms::Icon> icon;
    std::optional<comms::Color> color;

    [[nodiscard]] friend constexpr bool operator==(const KindEntryView&,
                                                   const KindEntryView&) noexcept = default;
};

namespace detail {

/// Overload set used to detect whether `T` is a tag-value kind and, if so,
/// which CRTP base it derives from. Relies on the standard rule that
/// template argument deduction from a derived class to a base-class
/// pointer succeeds when the base is a class template specialization
/// (see [temp.deduct.call]/4).
template <comms::FixedString Id, typename T>
constexpr KindCategory probe_kind(ClosedEnded<Id, T>*) noexcept {
    return KindCategory::Closed;
}

template <comms::FixedString Id, typename T>
constexpr KindCategory probe_kind(OpenEnded<Id, T>*) noexcept {
    return KindCategory::Open;
}

// Variadic ellipsis is intentional: it's the lowest-priority overload of
// the probe set, picked only when neither ClosedEnded nor OpenEnded matches.
// NOLINTNEXTLINE(cert-dcl50-cpp,modernize-avoid-variadic-functions)
constexpr std::monostate probe_kind(...) noexcept {
    return {};
}

/// Concept: T is a tag-value kind (derives from ClosedEnded or OpenEnded).
template <typename T>
concept TagValKind = !std::same_as<decltype(probe_kind(static_cast<T*>(nullptr))), std::monostate>;

/// Category of a tag-value kind. Only callable for types that satisfy
/// TagValKind.
template <typename T>
    requires TagValKind<T>
[[nodiscard]] constexpr KindCategory category_of() noexcept {
    return probe_kind(static_cast<T*>(nullptr));
}

}  // namespace detail

/// Type-erased handle to one registered kind. Stores `TagValDescriptor`,
/// `KindCategory`, and three function pointers captured at registration
/// time. No virtual dispatch, no allocation outside the explicit
/// `values()` snapshot.
class KindView {
public:
    using EmitFn = void (*)(void* user, const KindEntryView&);
    using WalkFn = void (*)(void* user, EmitFn emit);
    using FindFn = std::optional<KindEntryView> (*)(std::string_view code);

    constexpr KindView() noexcept = default;

    [[nodiscard]] constexpr TagValDescriptor descriptor() const noexcept {
        return descriptor_;
    }
    [[nodiscard]] constexpr std::string_view kind_id() const noexcept {
        return descriptor_.id;
    }
    [[nodiscard]] constexpr KindCategory category() const noexcept {
        return category_;
    }
    [[nodiscard]] constexpr bool is_closed_ended() const noexcept {
        return category_ == KindCategory::Closed;
    }
    [[nodiscard]] constexpr bool is_open_ended() const noexcept {
        return category_ == KindCategory::Open;
    }

    /// Allocating snapshot of the kind's current values, in declaration
    /// order (open-ended: predefined entries followed by extern entries in
    /// registration order).
    [[nodiscard]] std::vector<KindEntryView> values() const {
        std::vector<KindEntryView> out;
        for_each([&](const KindEntryView& e) { out.push_back(e); });
        return out;
    }

    /// Zero-allocation walk over the kind's current values. `f` is invoked
    /// per entry. Does not own `f`; reference / move-capture as appropriate.
    template <std::invocable<const KindEntryView&> F>
    void for_each(F&& f) const {
        using FRef = std::remove_reference_t<F>;
        auto trampoline =
            +[](void* user, const KindEntryView& e) { std::invoke(*static_cast<FRef*>(user), e); };
        // f is captured by reference via the void* user-data slot, so the
        // user-supplied callable doesn't need to be copy-constructible.
        walk_(static_cast<void*>(std::addressof(f)), trampoline);
    }

    /// Look up a value by code. Cheaper than calling values(): goes through
    /// the kind's own `try_of()` path.
    [[nodiscard]] std::optional<KindEntryView> find(const std::string_view code) const {
        return find_(code);
    }

    [[nodiscard]] friend constexpr bool operator==(const KindView& a, const KindView& b) noexcept {
        return a.descriptor_.id == b.descriptor_.id;
    }

private:
    template <typename T>
    friend class detail::KindViewFactory;

    TagValDescriptor descriptor_{};
    KindCategory category_{};
    WalkFn walk_ = nullptr;
    FindFn find_ = nullptr;
};

namespace detail {

/// Builds a KindView for a single concrete kind type. Captures the type
/// once and emits stateless function pointers that close over T.
template <typename T>
class KindViewFactory {
public:
    [[nodiscard]] static KindView build() {
        KindView v;
        v.descriptor_ = T::descriptor();
        v.category_ = category_of<T>();
        v.walk_ = &walk;
        v.find_ = &find;
        return v;
    }

private:
    static void walk(void* user, const KindView::EmitFn emit) {
        for (const auto& m : T::all_values()) {
            const KindEntryView kev{m.code, m.label, m.icon, m.color};
            emit(user, kev);
        }
    }

    static std::optional<KindEntryView> find(std::string_view code) {
        auto exp = T::try_of(code);
        if (!exp) {
            return std::nullopt;
        }
        return KindEntryView{exp->code(), exp->label(), exp->icon(), exp->color()};
    }
};

}  // namespace detail

/// Program-wide index of every kind that opted in via TAGVAL_REGISTER_KIND.
/// All methods are noexcept readers except `register_kind`, which mutates
/// shared storage and is only safe to call during static-init.
class KindRegistry {
public:
    /// Register a kind. Idempotent on `kind_id` — calling twice (e.g. the
    /// macro appears in two TUs that both end up in the binary) is a no-op
    /// on the second call.
    template <typename T>
        requires detail::TagValKind<T>
    static void register_kind() {
        auto& v = storage();
        const std::string_view id = T::kind_id();
        if (std::ranges::any_of(v, [&](const KindView& kv) { return kv.kind_id() == id; })) {
            return;
        }
        v.push_back(detail::KindViewFactory<T>::build());
    }

    /// All registered kinds, in registration order.
    [[nodiscard]] static std::span<const KindView> all() noexcept {
        return std::span<const KindView>{storage()};
    }

    /// Lazy filter view over the registered kinds that selects closed-ended
    /// kinds only. The result is a view; iterating it after a static-init
    /// registration that happened later in program startup is fine, since
    /// the view re-reads the underlying storage on each iteration.
    [[nodiscard]] static auto all_closed() noexcept {
        return all() | std::views::filter([](const KindView& kv) { return kv.is_closed_ended(); });
    }

    /// Lazy filter view over the registered kinds that selects open-ended
    /// kinds only.
    [[nodiscard]] static auto all_open() noexcept {
        return all() | std::views::filter([](const KindView& kv) { return kv.is_open_ended(); });
    }

    /// Find a kind by id. Returns nullptr if no kind with that id has been
    /// registered.
    [[nodiscard]] static const KindView* find(const std::string_view kind_id) noexcept {
        const auto& v = storage();
        const auto it =
            std::ranges::find_if(v, [&](const KindView& kv) { return kv.kind_id() == kind_id; });
        return it == v.end() ? nullptr : &*it;
    }

private:
    [[nodiscard]] static std::vector<KindView>& storage() {
        static std::vector<KindView> v;
        return v;
    }
};

}  // namespace tagval
