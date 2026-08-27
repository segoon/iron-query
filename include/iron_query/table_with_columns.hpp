#pragma once

#include <initializer_list>
#include <iron_query/column.hpp>
#include <iron_query/expr.hpp>
#include <iron_query/table.hpp>
#include <string>
#include <vector>

namespace iron_query {

/// @brief Table with associated columns. Usually not created by hands.
class [[nodiscard]] TableWithColumns final : public Table {
public:
  /// @brief Attaches a set of columns to a trusted, developer-written table
  /// name. Never pass untrusted/dynamic data as `name`.
  static TableWithColumns FromRaw(std::string name,
                                  std::vector<Column> columns);

  /// @brief Attaches a set of columns to a trusted, developer-written table
  /// name. Never pass untrusted/dynamic data as `name`.
  static TableWithColumns FromRaw(std::string name,
                                  std::initializer_list<Column> columns);

  /// @brief Builds a comma-separated Expr listing all column names, for use
  /// as a SELECT list.
  Expr SelectArgAll() const;

private:
  TableWithColumns(std::string name, std::vector<Column> columns);
  TableWithColumns(std::string name, std::initializer_list<Column> columns);

  std::vector<Column> columns_;
};

} // namespace iron_query
