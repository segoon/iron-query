#include "impl/identifier.hpp"
#include <iron_query/table_alias.hpp>

namespace iron_query {

TableAlias::TableAlias(std::string_view alias) : alias_(alias) {}

TableAlias TableAlias::From(std::string_view alias) {
  impl::ValidateIdentifier(alias);
  return TableAlias(alias);
}

std::string TableAlias::Dot(const std::string &column) const {
  return alias_ + "." + column;
}

std::string TableAlias::Dot(const Column &column) const {
  return Dot(column.name);
}

} // namespace iron_query
