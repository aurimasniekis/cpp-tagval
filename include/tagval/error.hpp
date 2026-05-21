#pragma once

/// @file
/// @brief tagval exception types and ParseError record used by try_of().

#include <stdexcept>
#include <string>
#include <string_view>

namespace tagval {

/// Common base exception. Catch this to handle any tagval-thrown error.
class TagValError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// Thrown by of() when a code is not present.
class UnknownCodeError : public TagValError {
public:
    using TagValError::TagValError;
};

/// Returned via std::expected from try_of() when a code is not present.
struct ParseError {
    std::string code;          ///< The attempted code.
    std::string_view kind_id;  ///< The kind it was looked up in.

    [[nodiscard]] std::string message() const {
        std::string msg = "tagval: unknown code '";
        msg += code;
        msg += "' for kind '";
        msg.append(kind_id);
        msg += "'";
        return msg;
    }
};

}  // namespace tagval
