#include "identifier.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <string_view>

namespace iron_query::impl {

bool IsPlainIdentifier(std::string_view s) {
  if (s.empty())
    return false;
  if (std::isalpha(static_cast<unsigned char>(s[0])) == 0 && s[0] != '_')
    return false;
  return std::all_of(s.begin() + 1, s.end(), [](char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
  });
}

// Validates `s` as a plain or dot-qualified SQL identifier, e.g. "name" or
// "schema.name".
void ValidateIdentifier(std::string_view s) {
  std::size_t start = 0;
  while (true) {
    std::size_t dot = s.find('.', start);
    std::string_view part = s.substr(start, dot - start);
    if (!IsPlainIdentifier(part))
      throw std::invalid_argument("iron_query: invalid identifier: " +
                                  std::string(s));
    if (dot == std::string_view::npos)
      break;
    start = dot + 1;
  }
}

namespace {

// PostgreSQL's operator-class character set; see
// https://www.postgresql.org/docs/current/sql-syntax-lexical.html#SQL-SYNTAX-OPERATORS
bool IsOperatorChar(char c) {
  static constexpr std::string_view kOperatorChars = "+-*/<>=~!@#%^&|`?";
  return kOperatorChars.find(c) != std::string_view::npos;
}

} // namespace

void ValidateOperatorName(std::string_view s) {
  if (s.empty() || !std::all_of(s.begin(), s.end(), IsOperatorChar) ||
      s.find("--") != std::string_view::npos ||
      s.find("/*") != std::string_view::npos)
    throw std::invalid_argument("iron_query: invalid operator name: " +
                                std::string(s));

  // A multi-character operator name ending in '+' or '-' must contain at
  // least one other operator-class character, or it would be ambiguous with
  // the lexer's own use of trailing +/- (same rule PostgreSQL enforces; a
  // single-character "+" or "-" is exempt).
  if (s.size() > 1 && (s.back() == '+' || s.back() == '-') &&
      s.find_first_not_of("+-") == std::string_view::npos)
    throw std::invalid_argument("iron_query: invalid operator name: " +
                                std::string(s));
}

} // namespace iron_query::impl
