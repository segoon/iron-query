#include <iron_query/column.hpp>
#include <iron_query/expr.hpp>

namespace iron_query {

Column::operator Expr() const { return Expr::FromRaw(name); }

Expr Column::ToExpr() const { return *this; }

// `alias`, not `name`: Column::name is a member.
SelectItem Column::As(std::string_view alias) const {
  return Expr(*this).As(alias);
}

} // namespace iron_query
