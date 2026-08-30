#include "impl/identifier.hpp"
#include <iron_query/exception.hpp>
#include <iron_query/expr.hpp>
#include <iron_query/table.hpp>
#include <iron_query/virtual_table.hpp>

namespace iron_query {

std::string VirtualTable::ToStringFormatted() const { return ToString(); }

std::string VirtualTable::ToStringBracketed() const {
  return "(" + ToString() + ")";
}

std::string VirtualTable::ToStringAsFromItem() const {
  // PostgreSQL rejects "FROM (SELECT ...)" with "subquery in FROM must have an
  // alias"; failing here turns that server-side error into a build-time one.
  throw LogicError("this FROM item requires an alias; use As()");
}

Table VirtualTable::As(std::string_view name) const {
  impl::ValidateIdentifier(name);
  // No outer brackets: PostgreSQL's table_ref grammar attaches the alias
  // directly to the item, and only ever allows parentheses around a bare
  // joined_table -- which is exactly what ToStringBracketed() produces here.
  return Table::FromRaw(ToStringBracketed() + " AS " + std::string(name));
}

VirtualTable::operator Expr() const && {
  return Expr::FromRaw(ToStringBracketed(), OperatorPrecedence::kSymbol);
}

} // namespace iron_query
