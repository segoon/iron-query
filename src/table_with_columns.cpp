#include "impl/identifier.hpp"
#include <algorithm>
#include <iron_query/exception.hpp>
#include <iron_query/operator_precedence.hpp>
#include <iron_query/table_with_columns.hpp>
#include <utility>

namespace iron_query {

TableWithColumns::TableWithColumns(std::string name,
                                   std::vector<Column> columns,
                                   std::string alias)
    : Table(std::move(name)), columns_(columns), alias_(std::move(alias)) {}

TableWithColumns::TableWithColumns(std::string name,
                                   std::initializer_list<Column> columns)
    : TableWithColumns(std::move(name), std::vector(columns)) {}

TableWithColumns TableWithColumns::FromRaw(std::string name,
                                           std::vector<Column> columns) {
  return TableWithColumns(std::move(name), std::move(columns));
}

TableWithColumns
TableWithColumns::FromRaw(std::string name,
                          std::initializer_list<Column> columns) {
  return TableWithColumns(std::move(name), columns);
}

Expr TableWithColumns::SelectArgAll() const {
  std::string s;
  for (const auto &col : columns_) {
    if (!s.empty())
      s += ", ";
    s += col.name;
  }
  return Expr::FromRaw(s);
}

TableWithColumns TableWithColumns::As(std::string_view name) const {
  impl::ValidateIdentifier(name);
  return TableWithColumns(ToStringBracketed() + " AS " + std::string(name),
                          columns_, std::string(name));
}

Expr TableWithColumns::Dot(const std::string &column_name) const {
  if (alias_.empty())
    throw LogicError("Dot() requires calling As() first");
  auto it =
      std::find_if(columns_.begin(), columns_.end(),
                   [&](const Column &c) { return c.name == column_name; });
  if (it == columns_.end())
    throw UnknownColumn(column_name, alias_);
  return Expr::FromRaw(alias_ + "." + column_name, OperatorPrecedence::kDot);
}

Expr TableWithColumns::Dot(const Column &column) const {
  return Dot(column.name);
}

} // namespace iron_query
