#pragma once

#include <string_view>

namespace iron_query::impl {

/// @brief Whether `s` is an unquoted SQL identifier, i.e. a letter or
/// underscore followed by letters, digits and underscores.
bool IsPlainIdentifier(std::string_view s);

/// @brief Validates `s` as a plain or dot-qualified SQL identifier.
/// @throws std::invalid_argument if it is neither.
void ValidateIdentifier(std::string_view s);

/// @brief Validates `s` as a syntactically valid PostgreSQL operator name
/// (https://www.postgresql.org/docs/current/sql-syntax-lexical.html#SQL-SYNTAX-OPERATORS):
/// non-empty, built only from operator-class characters, containing neither
/// `--` nor `/*` (which would start a comment), and not ending in `+`/`-`
/// unless it also contains another operator-class character.
/// @throws std::invalid_argument if `s` fails any of the above.
void ValidateOperatorName(std::string_view s);

} // namespace iron_query::impl
