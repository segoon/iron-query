#pragma once

#include <string_view>

namespace iron_query::impl {

/// @brief Whether `s` is an unquoted SQL identifier, i.e. a letter or
/// underscore followed by letters, digits and underscores.
bool IsPlainIdentifier(std::string_view s);

/// @brief Validates `s` as a plain or dot-qualified SQL identifier.
/// @throws std::invalid_argument if it is neither.
void ValidateIdentifier(std::string_view s);

} // namespace iron_query::impl
