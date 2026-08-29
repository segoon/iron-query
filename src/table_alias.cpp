#include "impl/identifier.hpp"
#include <iron_query/operator_precedence.hpp>
#include <iron_query/table_alias.hpp>

namespace iron_query {

TableAlias::TableAlias(std::string_view alias) : alias_(alias) {}

TableAlias TableAlias::From(std::string_view alias) {
  impl::ValidateIdentifier(alias);
  return TableAlias(alias);
}

Expr TableAlias::Dot(const std::string &column) const {
  return Expr::FromRaw(alias_ + "." + column, OperatorPrecedence::kDot);
}

Expr TableAlias::Dot(const Column &column) const { return Dot(column.name); }

} // namespace iron_query
