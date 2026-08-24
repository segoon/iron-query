#include <iron_query/table_with_columns.hpp>
#include <utility>

namespace iron_query {

TableWithColumns::TableWithColumns(std::string name,
                                   std::vector<Column> columns)
    : Table(std::move(name)), columns_(columns) {}

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

} // namespace iron_query
