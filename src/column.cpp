#include <iron_query/column.hpp>
#include <iron_query/expr.hpp>

namespace iron_query {

Column::operator Expr() const { return Expr::FromRaw(name); }

// `alias`, not `name`: Column::name is a member.
SelectItem Column::As(std::string_view alias) const {
  return Expr(*this).As(alias);
}

Condition Column::operator<(const Expr &other) const {
  return Expr(*this) < other;
}

Condition Column::operator<=(const Expr &other) const {
  return Expr(*this) <= other;
}

Condition Column::operator>(const Expr &other) const {
  return Expr(*this) > other;
}

Condition Column::operator>=(const Expr &other) const {
  return Expr(*this) >= other;
}

Condition Column::operator==(const Expr &other) const {
  return Expr(*this) == other;
}

Condition Column::operator!=(const Expr &other) const {
  return Expr(*this) != other;
}

} // namespace iron_query
