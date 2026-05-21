#pragma once

/// @file
/// @brief Optional nlohmann::json integration.
///
/// Activates when TAGVAL_WITH_NLOHMANN_JSON is defined, or when
/// <nlohmann/json.hpp> is on the include path.
///
/// Wire format: the bare code string.
///
///     Status::active()   <->   "active"
///
/// from_json on an unknown code throws tagval::UnknownCodeError — even for
/// open-ended kinds, deserialization never auto-creates an entry. The thrown
/// exception propagates out of `j.get<T>()` exactly like a json-library error,
/// but is the same exception type the rest of the tagval API throws, so a
/// single `catch (const tagval::UnknownCodeError&)` covers both call paths.

#if !defined(TAGVAL_WITH_NLOHMANN_JSON)
#if __has_include(<nlohmann/json.hpp>)
#define TAGVAL_WITH_NLOHMANN_JSON 1
#else
#define TAGVAL_WITH_NLOHMANN_JSON 0
#endif
#endif

#if TAGVAL_WITH_NLOHMANN_JSON

#include <tagval/base.hpp>
#include <tagval/error.hpp>

#include <nlohmann/json.hpp>

#include <concepts>
#include <string>
#include <string_view>

namespace tagval {

template <typename T>
    requires std::derived_from<T, detail::TagValBaseTag>
inline void to_json(::nlohmann::json& j, const T& v) {
    j = std::string{v.code()};
}

template <typename T>
    requires std::derived_from<T, detail::TagValBaseTag>
inline void from_json(const ::nlohmann::json& j, T& v) {
    const auto s = j.get<std::string>();
    auto exp = T::try_of(std::string_view{s});
    if (!exp) {
        throw UnknownCodeError{exp.error().message()};
    }
    v = *exp;
}

}  // namespace tagval

#endif  // TAGVAL_WITH_NLOHMANN_JSON
