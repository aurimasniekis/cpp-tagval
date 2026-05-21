# tagval

![CI](https://github.com/aurimasniekis/cpp-tagval/actions/workflows/ci.yml/badge.svg)

A header-only C++23 library for **tagged values** — strongly-typed open and
closed enumerations whose entries are first-class compile-time types with a
stable wire `code`, a human `label`, and optional UI metadata (`icon`,
`color`). `tagval` is what you reach for when `enum class` runs out of road:
each kind is a strongly-typed handle, each entry parses from a string,
formats to a string, hashes, and (optionally) round-trips through JSON or
[cpp-parcel](https://github.com/aurimasniekis/cpp-parcel).

## Why use this library?

`enum class` gives you compile-time identity and nothing else. `tagval` adds
the layers you usually end up reinventing by hand:

- **Stable wire codes.** The `code` survives serialization across versions
  and never collides with a label.
- **Human + UI metadata at the entry level.** Optional `label`, `icon`,
  `color` are attached to each entry as compile-time non-type template
  parameters — no runtime tables.
- **Parse from string.** `T::of("…")` throws on miss; `T::try_of("…")`
  returns `std::expected<T, ParseError>`.
- **Drop-in formatting.** `std::format("{}", v)`, `std::cout << v`,
  `std::hash<T>` and `std::unordered_set<T>` all work out of the box.
- **Plugin extensibility.** Open-ended kinds accept new entries declared in
  *other* translation units via `TAGVAL_EXTERN_ENTRY`, without touching the
  kind class.
- **Strong typing.** `DeviceKind == Status` doesn't compile; cross-kind
  handles can never silently coalesce.

Not the right fit if you need: locale-aware label resolution at the library
level, a numeric / bit-flag enumeration, runtime-defined kinds at the type
level, or many thousands of entries per kind (the registry is a linear
scan; see *Limitations*).

## Quick example

```cpp
#include <tagval/tagval.hpp>

#include <iostream>

class Status : public tagval::ClosedEnded<"status", Status> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Status, Active, active)
    TAGVAL_ENTRY(Status, Inactive, inactive, "Inactive")
    TAGVAL_ENTRY_AS(Status, Archived, is_archived, "archived", "Archived")

    using values_t = tagval::Values<Active, Inactive, Archived>;
};

int main() {
    std::cout << Status::active().code() << " — " << Status::active().label() << '\n';
    std::cout << Status::is_archived().code() << '\n';  // "archived"

    if (auto parsed = Status::try_of("inactive"); parsed) {
        std::cout << "parsed: " << *parsed << '\n';
    }

    try {
        (void)Status::of("nope");
    } catch (const tagval::UnknownCodeError& e) {
        std::cout << "rejected: " << e.what() << '\n';
    }
}
```

What's going on:

- `ClosedEnded<"status", Status>` is the CRTP base. The string literal
  `"status"` is the kind id and shows up in `kind_id()`, descriptors, and
  error messages.
- `TAGVAL_ENTRY(Owner, TypeName, FuncName, ...)` declares a nested `Entry`
  type and a static accessor of the same name. The wire `code` is the
  stringified function name (`"active"`). Trailing macro arguments fill the
  optional `Label`, `Icon`, `Color` parameters in that order.
- `TAGVAL_ENTRY_AS` is the same but takes an explicit code, letting you
  diverge from the accessor name (`is_archived()` returns the entry whose
  code is `"archived"`).
- `values_t = tagval::Values<...>` lists every entry. The list is read by
  `all_values()` and by `value<E>()` to `static_assert` membership.
- `of()` throws `UnknownCodeError` on a miss; `try_of()` returns
  `std::expected<Status, ParseError>` instead.

Every snippet in this README mirrors a file under `examples/` that CI
compiles and runs on every push — if a snippet drifts out of date, the
badge above will reflect that.

## Requirements

`tagval` requires a working **C++23** toolchain. The CI matrix in
`.github/workflows/ci.yml` runs the following on every push:

- Ubuntu (`ubuntu-latest`) with **GCC 14** — Debug and Release.
- Ubuntu (`ubuntu-latest`) with **Clang 20** — Debug.
- macOS (`macos-latest`) with the system **Apple Clang** — Debug and
  Release.
- ASan + UBSan run, clang-tidy run (on macOS), and a clang-format-22 check.

Other compilers that implement the C++23 features the library uses
(`concepts`, `std::expected`, inline-variable templates, ranges, NTTP
class types) are expected to work but are not gated by CI. MSVC in
particular is not exercised — see *Limitations* for the static-archive
caveat that affects it.

The library has no required runtime dependencies. Optional integrations:

- [`nlohmann/json`](https://github.com/nlohmann/json) ≥ 3.12 — JSON
  adapter.
- [`cpp-parcel`](https://github.com/aurimasniekis/cpp-parcel) — `TagValCell`
  envelope.

Both adapters are auto-detected via `__has_include`, so simply having the
headers visible to the preprocessor is enough.

## Installation

### CMake — FetchContent

```cmake
cmake_minimum_required(VERSION 3.25)
project(my_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(tagval
    URL      https://github.com/aurimasniekis/cpp-tagval/archive/refs/tags/v0.1.0.tar.gz
    URL_HASH SHA256=0000000000000000000000000000000000000000000000000000000000000000
)
FetchContent_MakeAvailable(tagval)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE tagval::tagval)
```

### CMake — find_package (after `cmake --install`)

```cmake
find_package(tagval 0.1 REQUIRED)
target_link_libraries(my_app PRIVATE tagval::tagval)
```

Install rules are skipped automatically if `nlohmann_json` or `parcel` came
from `FetchContent` — those can't be re-exported. Disable
`TAGVAL_WITH_NLOHMANN_JSON` / `TAGVAL_WITH_PARCEL` or supply them via
`find_package` to re-enable installation.

### Meson

```meson
tagval_dep = dependency('tagval', version: '>=0.1.0',
    fallback: ['tagval', 'tagval_dep'])
```

`meson.options` exposes the same toggles (`tests`, `examples`, `json`,
`parcel`).

### Header-only drop-in

Copy `include/tagval/` onto your include path and add it to your
compiler's `-I` flags. The JSON and Parcel adapters auto-activate as soon
as the relevant third-party headers are on the include path, so no
preprocessor flags are required when copying.

## Build options

Toggles understood by both CMake and Meson (Meson uses lowercase, no
prefix):

| CMake option                | Default   | What it does                                             |
|-----------------------------|-----------|----------------------------------------------------------|
| `TAGVAL_BUILD_TESTS`        | top-level | Build the GoogleTest suite.                              |
| `TAGVAL_BUILD_EXAMPLES`     | top-level | Build the example binaries.                              |
| `TAGVAL_BUILD_DOCS`         | OFF       | Build Doxygen HTML docs (`make docs`).                   |
| `TAGVAL_WITH_NLOHMANN_JSON` | ON        | Link nlohmann/json; enable `<tagval/json_nlohmann.hpp>`. |
| `TAGVAL_WITH_PARCEL`        | ON        | Link cpp-parcel; enable `<tagval/parcel.hpp>`.           |
| `TAGVAL_ENABLE_SANITIZERS`  | OFF       | ASan + UBSan in Debug builds.                            |
| `TAGVAL_ENABLE_CLANG_TIDY`  | OFF       | Run clang-tidy during the build.                         |
| `TAGVAL_ENABLE_COVERAGE`    | OFF       | Clang source-based coverage.                             |
| `TAGVAL_WARNINGS_AS_ERRORS` | top-level | Treat compiler warnings as errors.                       |
| `TAGVAL_INSTALL`            | top-level | Generate install rules.                                  |

The `Makefile` is a thin wrapper around common workflows: `make test`,
`make sanitize`, `make tidy`, `make release`, `make coverage`,
`make docs`, `make no-json`, `make no-parcel`, `make ci` (the full
pre-push gate), `make format`, `make format-check`. Run `make help` for
the full list.

## Granular includes

`<tagval/tagval.hpp>` is an umbrella that pulls in every public header.
If you'd rather keep a translation unit lean, include only what you use:

| Feature                                                          | Header                            |
|------------------------------------------------------------------|-----------------------------------|
| `ClosedEnded` base                                               | `<tagval/closed_ended.hpp>`       |
| `OpenEnded` base                                                 | `<tagval/open_ended.hpp>`         |
| `TAGVAL_ENTRY*` macros                                           | `<tagval/macros.hpp>`             |
| `tagval::Entry` / `TagValMetadata` / `metadata_v`                | `<tagval/entry.hpp>`              |
| `tagval::Values<…>`                                              | `<tagval/values.hpp>`             |
| `tagval::OpenEndedRegistry<Owner>` (extern entries for one kind) | `<tagval/openended_registry.hpp>` |
| `tagval::KindRegistry` (program-wide kind index)                 | `<tagval/kind_registry.hpp>`      |
| `tagval::TagValDescriptor`                                       | `<tagval/descriptor.hpp>`         |
| `tagval::fixed_string`                                           | `<tagval/fixed_string.hpp>`       |
| Exception types                                                  | `<tagval/error.hpp>`              |
| `std::format` integration                                        | `<tagval/format.hpp>`             |
| `std::ostream` integration                                       | `<tagval/ostream.hpp>`            |
| `std::hash` specialization                                       | `<tagval/hash.hpp>`               |
| `nlohmann::json` adapter                                         | `<tagval/json_nlohmann.hpp>`      |
| `cpp-parcel` adapter                                             | `<tagval/parcel.hpp>`             |
| Version macros                                                   | `<tagval/version.hpp>`            |

## Core concepts

### Handles

A *handle* is your kind class — `Status`, `DeviceKind`, etc. It derives
from either `tagval::ClosedEnded<Id, Self>` or `tagval::OpenEnded<Id, Self>`
via CRTP. The handle's runtime data is a single `const TagValMetadata*`,
so handles are trivially copyable and cheap to pass around. A
default-constructed handle is *empty*: `empty() == true`,
`static_cast<bool>(h) == false`, and `code()` / `label()` return empty
views.

### Entries

An *entry* is a distinct type per value. The `TAGVAL_ENTRY` family of
macros declares one:

```cpp
TAGVAL_ENTRY    (Status, Active,   active)                           // code = "active"
TAGVAL_ENTRY    (Status, Inactive, inactive, "Inactive", "mdi:off")  // + label, + icon
TAGVAL_ENTRY_AS (Status, Archived, is_archived, "archived", "Archived")
```

Each declaration expands to a nested `struct` deriving from
`tagval::Entry<Owner, Code, Label, Icon, Color>` plus a static accessor.
The accessor returns a `const Status&` referencing a function-local-static
handle, so its address is stable.

### `Values<…>`

The compile-time list of entries:

```cpp
using values_t = tagval::Values<Active, Inactive, Archived>;
```

`Values<…>` static-asserts that every entry's owner is the same type and
that no two entries share a `code`. `ClosedEnded::value<E>()` further
static-asserts that `E` is in this list.

### Metadata views

`tagval::TagValMetadata` is the runtime view of an entry — four
`std::string_view`s (`code`, `label`, `icon`, `color`) that point into the
entry's NTTP storage, so they're valid for the lifetime of the program. An
empty `Label` falls back to `code`, so `label()` is never empty for a
valid handle. The pinned metadata constant is exposed as
`tagval::metadata_v<E>`.

### Per-kind extern-entry registry (`OpenEndedRegistry`)

`tagval::OpenEndedRegistry<Owner>` is a per-kind list of metadata
pointers used by `OpenEnded` to merge the compile-time `values_t` entries
with any extern entries contributed at static-init time. Predefined
entries are seeded lazily on first use; extern registrars deduplicate by
code. `ClosedEnded` kinds do not use this registry — their values come
from a `constexpr static` array materialized from `values_t`.

### Global kind registry (`KindRegistry`)

`tagval::KindRegistry` is a separate, opt-in, program-wide index of
*kinds* (not entries). Place `TAGVAL_REGISTER_KIND(MyKind)` at namespace
scope to add a kind. Once registered, the kind is discoverable through
`KindRegistry::all()`, `KindRegistry::all_closed()`,
`KindRegistry::all_open()`, and `KindRegistry::find(kind_id)`. Each
result is a `KindView` that exposes the kind's `descriptor()`,
`category()`, a `values()` snapshot, a zero-allocation `for_each(F)`
walk, and a code-based `find()`. Intended for documentation generators
and other introspection tools.

```cpp
#include <tagval/tagval.hpp>

class Status : public tagval::ClosedEnded<"status", Status> { /* … */ };
class DeviceKind : public tagval::OpenEnded<"device_kind", DeviceKind> { /* … */ };

TAGVAL_REGISTER_KIND(Status);
TAGVAL_REGISTER_KIND(DeviceKind);

void emit_docs() {
    for (const auto& kv : tagval::KindRegistry::all_closed()) {
        std::cout << kv.kind_id() << " (closed)\n";
        kv.for_each([](const tagval::KindEntryView& e) {
            std::cout << "  " << e.code << " — " << e.label << '\n';
        });
    }
}
```

### Descriptors

`tagval::TagValDescriptor` is the runtime view of *kind*-level metadata —
`id`, `name`, `icon`, `color`. It's always available via
`Status::descriptor()` (with at least `id` filled in from the kind id);
opt into the rest by defining `static constexpr make_descriptor()`.

## Closed-ended kinds

A *closed-ended* kind fixes its value set at compile time. Unknown codes
never parse, and `value<E>()` ill-formedly refers to entries you forgot to
list.

```cpp
#include <tagval/tagval.hpp>

#include <iostream>

class Status : public tagval::ClosedEnded<"status", Status> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    static constexpr tagval::TagValDescriptor make_descriptor() noexcept {
        return tagval::TagValDescriptor{.id = "status", .name = "Status"};
    }

    TAGVAL_ENTRY(Status, Active,   active)
    TAGVAL_ENTRY(Status, Inactive, inactive,    "Inactive")
    TAGVAL_ENTRY_AS(Status, Archived, is_archived, "archived", "Archived")

    using values_t = tagval::Values<Active, Inactive, Archived>;
};

int main() {
    std::cout << "kind: " << Status::kind_id() << '\n';
    for (const auto& m : Status::all_values()) {
        std::cout << "  - " << m.code << " (" << m.label << ")\n";
    }

    std::cout << "of('active'): " << Status::of("active") << '\n';

    try {
        (void)Status::of("nonsense");
    } catch (const tagval::UnknownCodeError& e) {
        std::cout << "rejected: " << e.what() << '\n';
    }
}
```

Notes worth knowing:

- `Status::value<Status::Active>() == Status::active()` — both resolve to
  the same `TagValMetadata` record.
- `Status::all_values()` returns a `std::span<const TagValMetadata>`
  pointing into a `constexpr static` array — valid for the program's
  lifetime.
- Calling `Status::value<E>()` with a stray entry whose owner is `Status`
  but which is missing from `values_t` is a `static_assert` failure, not
  a runtime miss.
- The empty `Inactive` icon trick: `TAGVAL_ENTRY(..., "Inactive")` skips
  the icon/color fields. Trailing fields default to empty strings, so
  `.icon()` and `.color()` return `""`.

## Open-ended kinds

An *open-ended* kind has the same in-class declaration shape — `TAGVAL_ENTRY`
plus `values_t` — and *also* accepts entries from other translation units.
Useful for plugin systems where the host knows a built-in set and vendors
extend it without recompiling the kind class.

```cpp
#include <tagval/tagval.hpp>

#include <iostream>

class DeviceKind : public tagval::OpenEnded<"device_kind", DeviceKind> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    static constexpr tagval::TagValDescriptor make_descriptor() noexcept {
        return tagval::TagValDescriptor{.id = "device_kind", .name = "Device Kind"};
    }

    TAGVAL_ENTRY(DeviceKind, Phone,  phone,  "Phone")
    TAGVAL_ENTRY(DeviceKind, Tablet, tablet, "Tablet")
    TAGVAL_ENTRY(DeviceKind, Laptop, laptop, "Laptop")

    using values_t = tagval::Values<Phone, Tablet, Laptop>;
};

int main() {
    std::cout << "All " << DeviceKind::descriptor().name << ":\n";
    for (const auto& m : DeviceKind::all_values()) {
        std::cout << "  - " << m.code << " (" << m.label << ")\n";
    }
    std::cout << std::boolalpha
              << (DeviceKind::of("phone") == DeviceKind::phone()) << '\n';  // true
}
```

`all_values()` for an open-ended kind is a `std::ranges::transform_view`
over the runtime registry. The element type is still
`const TagValMetadata&`, so range-based for loops and
`std::ranges::any_of` work unchanged. The first call seeds the registry
with the `values_t` entries.

## Plugin / extern entries

To extend an open-ended kind from another TU (or another library), declare
entries at namespace scope with `TAGVAL_EXTERN_ENTRY` /
`TAGVAL_EXTERN_ENTRY_AS`:

```cpp
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
    for (const auto& m : Plugin::all_values()) {
        std::cout << "  - " << m.code << " (" << m.label << ")\n";
    }
    std::cout << (Plugin::of("smart_watch") == vendor_a::smart_watch()) << '\n';  // 1
}
```

What the extern macros do:

- Declare a nested entry struct (same as the in-class form).
- Emit an `inline` accessor function in the surrounding namespace.
- Emit an `inline` registrar variable whose initializer calls
  `Registry<Owner>::add(&metadata_v<E>)` at static-init time. The variable
  is marked `[[gnu::used]]` so GCC and Clang (including Apple Clang) keep
  it in the binary even when nothing else in the TU is referenced.

`TAGVAL_EXTERN_ENTRY` derives the code from the function name;
`TAGVAL_EXTERN_ENTRY_AS` takes an explicit code. Registry adds are
idempotent on `->code`, so racing with a redeclaration is harmless.

**Watch out for the static-archive case.** If your vendor TUs are packed
into a `.a` and nothing else in those TUs is referenced from the
consumer, the linker skips the archive members entirely and the registrar
never runs. See *Limitations* below.

## Kind descriptor

Define `static constexpr make_descriptor()` to attach kind-level
metadata. Without it, `descriptor()` still returns `{id = Id}` with the
other fields empty.

```cpp
#include <tagval/tagval.hpp>

#include <iostream>

class Severity : public tagval::ClosedEnded<"severity", Severity> {
public:
    using base_t = ClosedEnded;
    using base_t::base_t;

    static constexpr tagval::TagValDescriptor make_descriptor() noexcept {
        return tagval::TagValDescriptor{
            .id    = "severity",
            .name  = "Alert Severity",
            .icon  = "alert",
            .color = "#aa0000",
        };
    }

    TAGVAL_ENTRY(Severity, Info,  info,  "Info",  "info",    "#3366cc")
    TAGVAL_ENTRY(Severity, Warn,  warn,  "Warn",  "warning", "#cc9900")
    TAGVAL_ENTRY(Severity, Error, error, "Error", "error",   "#cc0000")

    using values_t = tagval::Values<Info, Warn, Error>;
};

int main() {
    constexpr auto k = Severity::descriptor();
    std::cout << "Kind: " << k.id << " — " << k.name << " (icon=" << k.icon << ")\n";

    for (const auto& [code, label, icon, color] : Severity::all_values()) {
        std::cout << "  [" << icon << "] " << code << " (" << label << ") " << color << '\n';
    }
}
```

`TagValDescriptor` stores `std::string_view`s, so the storage you point
at must outlive the descriptor. Returning string literals (as here) is
always safe.

## Parsing, formatting, comparing

```cpp
Status::of("active");                    // Status — throws UnknownCodeError on miss
Status::try_of("nope");                  // std::expected<Status, ParseError>
Status::try_of("nope").error().message();
// → "tagval: unknown code 'nope' for kind 'status'"

std::format("{}", Status::active());     // "active"
std::cout << Status::active();           // active

Status::active() == Status::of("active");  // true
Status::active()  < Status::inactive();    // true — lexicographic on code()

Status empty;
std::format("{}", empty);                // ""
empty < Status::active();                // true — empty sorts before populated
```

`std::format` accepts only the default spec (`"{}"`); anything else
(`"{:>10}"`, `"{:.5}"`) throws `std::format_error`. `operator<<` writes
the bare `code()`. `operator<=>` returns `std::strong_ordering`, so
handles drop straight into `std::set`, `std::map`, and `std::ranges::sort`.

## Hashing & containers

```cpp
#include <tagval/tagval.hpp>

#include <unordered_set>

std::unordered_set<DeviceKind> seen{DeviceKind::phone(), DeviceKind::tablet()};
seen.contains(DeviceKind::of("phone"));  // true
```

`std::hash<T>` hashes `(kind_id, code)` so equal handles within a process
share a hash and cross-kind handles never collide. Hash values are **not**
stable across processes — the underlying `std::hash<std::string_view>` is
implementation-defined and may be salted per-run. Use it for in-memory
containers, not for persistent fingerprints.

## JSON support (optional)

Activates when `<nlohmann/json.hpp>` is on the include path (or when
`TAGVAL_WITH_NLOHMANN_JSON=1` is defined explicitly, which CMake does for
you when the option is on).

```cpp
#include <tagval/tagval.hpp>

#include <nlohmann/json.hpp>

#include <iostream>

class TransactionType : public tagval::OpenEnded<"tx_type", TransactionType> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(TransactionType, Debit,  debit,  "Debit")
    TAGVAL_ENTRY(TransactionType, Credit, credit, "Credit")

    using values_t = tagval::Values<Debit, Credit>;
};

int main() {
    const nlohmann::json j = TransactionType::debit();  // "debit"
    const auto recovered = j.get<TransactionType>();
    std::cout << recovered << '\n';                     // debit

    try {
        (void)nlohmann::json("nonsense").get<TransactionType>();
    } catch (const tagval::UnknownCodeError& e) {
        std::cout << "rejected: " << e.what() << '\n';
    }
}
```

Wire format is the bare code string. `from_json` on an unknown code
throws `tagval::UnknownCodeError` — **even for open-ended kinds**.
Deserialization never auto-creates entries; add them through the macros
instead. The same exception type covers `of()` and `j.get<T>()`, so a
single `catch` handles both call paths.

## Parcel support (optional)

Activates when `<parcel/parcel.h>` is on the include path (or when
`TAGVAL_WITH_PARCEL=1` is defined explicitly).

```cpp
#include <tagval/tagval.hpp>

#include <parcel/parcel.h>

#include <iostream>

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
    const auto j = cell.to_json();                     // {"k":"tagval","v":"card"}

    ::parcel::ParcelRegistry reg;
    reg.register_cells<Cell>();

    const auto decoded = reg.cell_from_json(j);
    if (const auto* typed = dynamic_cast<Cell*>(decoded.get()); typed != nullptr) {
        std::cout << "decoded: " << typed->value << '\n';
    }
}
```

**Limitation.** Every `TagValCell<T>` instantiation reports
`kind_id = "tagval"`, so a single `ParcelRegistry` cannot dispatch by
kind to multiple `TagT`. This matches cpp-parcel's documented constraint
for site-knows-the-type usage; if the site really does know the type, the
inner JSON adapter's `try_of()` still catches cross-kind mismatches with
non-overlapping codes.

## Error handling

All exceptions thrown by `tagval` derive from `tagval::TagValError`, which
itself derives from `std::runtime_error`. A single `catch (const
tagval::TagValError&)` handles any error the library raises.

| Mechanism       | When                                                                                                                        | Type                                                |
|-----------------|-----------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------|
| `throw`         | `T::of(code)` miss; `from_json` miss; non-default `std::format` spec (raises `std::format_error`)                           | `tagval::UnknownCodeError` (or `std::format_error`) |
| `std::expected` | `T::try_of(code)`                                                                                                           | `std::expected<T, tagval::ParseError>`              |
| `static_assert` | duplicate code in `Values<…>`; empty code in `Entry`; `value<E>()` where `E` is missing from `values_t`; missing `values_t` | compile-time                                        |

```cpp
auto exp = Status::try_of("nope");
if (!exp) {
    const tagval::ParseError& e = exp.error();
    std::cout << e.message() << '\n';   // "tagval: unknown code 'nope' for kind 'status'"
    std::cout << e.code << '\n';        // "nope"
    std::cout << e.kind_id << '\n';     // "status"
}
```

`ParseError::message()` formats the same string `UnknownCodeError::what()`
exposes, so callers can use either style without rewriting the message.

## Edge cases and pitfalls

- **Empty (default-constructed) handles.** `Status s;` is well-formed.
  `s.empty()` is true, `static_cast<bool>(s)` is false, and `s.code()` /
  `s.label()` return empty views. Formatting prints `""`; ordering puts
  empties before any populated handle. Compare against another empty
  handle with `==`.
- **Duplicate codes.** `tagval::Values<Active, Active>` or two entries
  with the same `Code` string fail to compile via `static_assert` in
  `Values<…>` — no ambiguous runtime miss.
- **Empty codes.** `tagval::Entry<Owner, "">` fails to compile via
  `static_assert` in `Entry`.
- **Cross-kind comparison.** `KindA::x() == KindB::x()` is ill-formed by
  design; cross-kind values can never silently coalesce. If you really
  need to compare across kinds, compare `kind_id()` and `code()`
  explicitly.
- **Hash is per-process.** Don't persist `std::hash<T>(v)` to disk or
  send it over the wire — use `code()` (and `kind_id()`) instead.
- **Static-init ordering across translation units.** Extern entries
  register themselves at static-init time. Looking one up from another
  static initializer that runs *before* the registrar is undefined; defer
  such lookups to function bodies (called after `main()` begins, or via
  Meyers singletons). Within a single TU, ordering is well-defined.
- **Static archives drop the registrar.** Both
  `TAGVAL_EXTERN_ENTRY` and `TAGVAL_REGISTER_KIND` emit an inline
  `[[gnu::used]]` variable whose initializer runs at static-init. The
  attribute keeps the variable from being dead-stripped *after* the
  object is linked in; it does **not** override the archive selector. If
  the TU containing the registrar is only inside a `.a` and nothing else
  in that TU is referenced from the consumer, the linker skips the
  entire archive member and the registrar never runs — the entry or
  kind silently fails to appear. Workarounds: link the registrar
  objects directly (CMake `OBJECT` library / Meson source list),
  reference one symbol per registrar TU from the consumer, or use
  `-Wl,--whole-archive` (GNU) / `-Wl,-force_load` (Apple). MSVC
  additionally needs `/INCLUDE:<mangled-name>` to keep the inline
  registrar; CI does not exercise MSVC.
- **`from_json` never auto-creates entries.** Open-ended kinds still
  reject unknown codes on deserialization. Add entries through
  `TAGVAL_EXTERN_ENTRY`.
- **`std::format` is strict.** Only `"{}"` is accepted; `"{:>10}"`,
  `"{:.3}"`, etc. throw `std::format_error`.
- **Registry is not thread-safe.** `Registry<Owner>::add()` mutates a
  per-kind vector and is only safe during static-init (single-threaded by
  contract). After `main()` begins the registry is read-only and
  concurrent reads from any thread are fine.
- **TagValDescriptor string lifetime.** The descriptor stores
  `std::string_view`s. If `make_descriptor()` returns views into
  function-local storage, those views dangle. Use string literals.

## API overview

The umbrella `<tagval/tagval.hpp>` exposes the following public surface
under `namespace tagval`:

| Symbol                                      | Purpose                                          | Notes                                                               |
|---------------------------------------------|--------------------------------------------------|---------------------------------------------------------------------|
| `ClosedEnded<Id, Self>`                     | CRTP base for fixed-set kinds                    | `value<E>()`, `of`, `try_of`, `all_values`, `kind_id`, `descriptor` |
| `OpenEnded<Id, Self>`                       | CRTP base for plugin-extensible kinds            | Same surface; registry-backed                                       |
| `Entry<Owner, Code, Label, Icon, Color>`    | Compile-time entry record                        | Subclass via `TAGVAL_ENTRY*`                                        |
| `Values<E…>`                                | Compile-time list of entries                     | Static-asserts owner + code uniqueness                              |
| `OpenEndedRegistry<Owner>`                  | Per-kind runtime registry of extern entries      | Mutate only at static-init                                          |
| `KindRegistry`                              | Program-wide index of registered kinds           | Opt-in via `TAGVAL_REGISTER_KIND`                                   |
| `KindView`                                  | Type-erased handle to one registered kind        | `descriptor()`, `category()`, `values()`, `for_each()`, `find()`    |
| `KindCategory`                              | Closed / Open enum                               | Returned by `KindView::category()`                                  |
| `TagValMetadata`                            | Runtime view of an entry (code/label/icon/color) | Pointers stable for program lifetime                                |
| `TagValDescriptor`                          | Runtime view of kind-level metadata              | Provided by `descriptor()`                                          |
| `TagValError`, `UnknownCodeError`           | Exception types                                  | Derive from `std::runtime_error`                                    |
| `ParseError`                                | `try_of` failure record                          | Has `code`, `kind_id`, `message()`                                  |
| `fixed_string<N>`                           | NTTP-friendly string class                       | Used for kind id and entry code                                     |
| `metadata_v<E>`                             | Pinned `TagValMetadata` constant for entry `E`   | ODR-merged across TUs                                               |
| `TagValCell<TagT>` (optional)               | cpp-parcel envelope                              | `kind_id = "tagval"`                                                |
| `TAGVAL_ENTRY[_AS]`                         | In-class entry macro                             | Derived or explicit code                                            |
| `TAGVAL_EXTERN_ENTRY[_AS]`                  | Extern entry macro                               | Registers an entry into `OpenEndedRegistry<Owner>` at static-init   |
| `TAGVAL_REGISTER_KIND(K)`                   | Kind-registration macro                          | Adds a kind to `KindRegistry` at static-init                        |
| `TAGVAL_VERSION_{MAJOR,MINOR,PATCH,STRING}` | Header version macros                            | `<tagval/version.hpp>`                                              |

Internal helpers under `tagval::detail` (e.g. `HandleBase`,
`TagValBaseTag`) are not part of the public API and may change between
patch releases.

## Examples

The `examples/` directory is built by `make examples` and run on every
CI push:

| Example                           | Demonstrates                                                     |
|-----------------------------------|------------------------------------------------------------------|
| `examples/closed_ended.cpp`       | A `ClosedEnded` kind end-to-end: declarations, `of`, descriptor. |
| `examples/open_ended.cpp`         | An `OpenEnded` kind with predefined entries only.                |
| `examples/extern_entries.cpp`     | Plugin entries via `TAGVAL_EXTERN_ENTRY[_AS]`.                   |
| `examples/metadata.cpp`           | Kind-level `make_descriptor()` plus per-entry icon/color.        |
| `examples/formatting.cpp`         | `std::format` and `operator<<`.                                  |
| `examples/json_integration.cpp`   | nlohmann/json round-trip + error path.                           |
| `examples/parcel_integration.cpp` | cpp-parcel cell round-trip.                                      |

## Testing

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Or simply `make test`. The suite includes `tagval_test_extern_split`,
which compiles two vendor TUs into a CMake `OBJECT` library and verifies
that `[[gnu::used]]` keeps the registrar symbols alive — a regression
test for the static-archive caveat above.

Run the full pre-push gate (the same checks CI runs) with `make ci`. The
clang-format step requires **clang-format-22** specifically; older
versions may produce a different diff.

## Limitations

- **Linear-scan lookup.** `Registry<Owner>` and `ClosedEnded::all_values()`
  use linear scans for `of()` / `try_of()`. For typical kinds (a handful
  to a few dozen entries) this is faster than hashing. Add a hash
  side-index if you genuinely have many hundreds of values.
- **Cross-kind comparison is rejected by design.** `DeviceKind ==
  Status` doesn't compile; compare `kind_id()` and `code()` explicitly
  if you need the looser semantics.
- **`TagValCell<T>` shares `kind_id="tagval"`** across all
  instantiations, so a single `ParcelRegistry` can't dispatch by kind to
  multiple `TagT`. Site-knows-the-type usage is fine; multi-kind
  dispatch isn't.
- **Static-init ordering caveat** for extern entries: lookups from
  another static initializer that runs before the registrar are
  undefined. Defer to function bodies.
- **Static-archive linker culling.** GCC/Clang `[[gnu::used]]` keeps the
  registrar alive *within* a linked-in object; it doesn't force the
  archive selector to pull the object in. MSVC needs `/INCLUDE:` and is
  not exercised by CI.

## FAQ

**Is the library header-only?** Yes — including `<tagval/tagval.hpp>`
(or any individual header) is all you need at compile time. There's no
`tagval.cpp` and no precompiled binary. The CMake `tagval` target is an
`INTERFACE` library that only carries include paths and the optional
dependency links.

**What happens if a code is invalid?** `T::of("...")` throws
`tagval::UnknownCodeError`; `T::try_of("...")` returns
`std::expected<T, ParseError>` with the error filled in. Duplicate or
empty codes are rejected at compile time via `static_assert`.

**Can I use it in multiple threads?** Reads (`code`, `label`, `icon`,
`color`, `of`, `try_of`, `all_values`) are safe to call concurrently.
The registry's mutating path (extern registration) is single-threaded
by contract — it only runs at static-init.

**Does the handle own its strings?** No. Handles hold a pointer to a
`TagValMetadata` whose `string_view`s point into the entry's NTTP
storage. That storage lives for the program's lifetime, so handles are
trivially copyable and can be passed by value freely.

**Which compilers are supported?** GCC 14, Clang 20, and Apple Clang
(macOS-latest) are gated by CI on every push. Other C++23-conformant
compilers should work; MSVC requires manual `/INCLUDE:` linker
arguments for extern entries in static archives and is not exercised
by CI.

**My extern entry isn't showing up in `all_values()`. What's wrong?**
Almost always the static-archive case described under *Limitations*:
the linker culled the entire `.o` containing the registrar because no
other symbol in it was referenced from the consumer. Switch the vendor
sources to a CMake `OBJECT` library (or Meson source list) so the
objects are linked directly, or pass `-Wl,--whole-archive` / `-Wl,-force_load`.

**How do I debug build errors?** The most common compile-time errors
have static-assert messages with `tagval:` in them — search the build
log for that prefix. The library uses concept-constrained
specializations for `std::hash`, `std::formatter`, and `operator<<`, so
unrelated overload sets aren't polluted.

## Contributing

Contributions to the library are welcome! If you encounter any issues or have suggestions for
improvements,
please feel free to submit a pull request or open an issue on the project's repository.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
