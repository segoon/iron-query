#pragma once

#include <initializer_list>
#include <iron_query/column.hpp>
#include <iron_query/expr.hpp>
#include <iron_query/table.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace iron_query {

/// @brief Table with associated columns. Usually not created by hands.
/// @ingroup schema
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

  /// @brief Aliases this table as `this AS name`, preserving its column list
  /// so @ref Dot can verify a referenced column actually belongs to this
  /// table. Hides @ref VirtualTable::As, which would otherwise discard the
  /// column list.
  /// @throws InvalidIdentifier if `name` is not a valid identifier.
  TableWithColumns As(std::string_view name) const;

  /// @brief Qualifies `column_name` as `alias.column_name`.
  /// @throws LogicError if called before @ref As.
  /// @throws UnknownColumn if `column_name` is not one of this table's
  /// columns.
  Expr Dot(const std::string &column_name) const;

  /// @brief Qualifies `column` as `alias.column.name`.
  /// @throws LogicError if called before @ref As.
  /// @throws UnknownColumn if `column` is not one of this table's columns.
  Expr Dot(const Column &column) const;

private:
  TableWithColumns(std::string name, std::vector<Column> columns,
                   std::string alias = "");
  TableWithColumns(std::string name, std::initializer_list<Column> columns);

  std::vector<Column> columns_;
  std::string alias_;
};

} // namespace iron_query
