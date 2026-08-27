#include "identifier.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

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

} // namespace iron_query::impl
