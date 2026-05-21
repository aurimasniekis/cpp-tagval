#pragma once

/// @file
/// @brief operator<< for any tagval value type — writes value.code() to the stream.

#include <tagval/base.hpp>

#include <concepts>
#include <ostream>

namespace tagval {

template <typename T>
    requires std::derived_from<T, detail::TagValBaseTag>
inline std::ostream& operator<<(std::ostream& os, const T& v) {
    return os << v.code();
}

}  // namespace tagval
