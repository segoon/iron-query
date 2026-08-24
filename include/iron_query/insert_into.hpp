#pragma once

#include <initializer_list>
#include <iron_query/expr.hpp>
#include <iron_query/table.hpp>
#include <string>
#include <vector>

namespace iron_query {

/// @brief Transitional representation for INSERT INTO query
class [[nodiscard]] InsertInto final {
public:
  /// @brief Starts an `INSERT INTO tbl` query.
  InsertInto(const Table &tbl);

  /// @brief Sets the list of columns to insert into, replacing any previously
  /// set list.
  /// @throws std::logic_error if the count does not match an already-set
  /// @ref Values list.
  InsertInto Columns(std::initializer_list<Expr> cols) &&;

  /// @brief Sets the list of values to insert, matching @ref Columns by
  /// position and replacing any previously set list.
  /// @throws std::logic_error if the count does not match an already-set
  /// @ref Columns list.
  InsertInto Values(std::initializer_list<Expr> vals) &&;

  /// @throws std::logic_error if no columns or no values were set, or if the
  /// two lists have different lengths.
  std::string ToString() const;

private:
  std::string into_;
  std::vector<std::string> columns_;
  std::vector<std::string> values_;
};

} // namespace iron_query
