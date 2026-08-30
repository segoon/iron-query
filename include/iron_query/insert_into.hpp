#pragma once

#include <initializer_list>
#include <iron_query/expr.hpp>
#include <iron_query/select_item.hpp>
#include <iron_query/table.hpp>
#include <string>
#include <utility>
#include <vector>

namespace iron_query {

/// @brief Transitional representation for INSERT INTO query
class [[nodiscard]] InsertInto final {
public:
  /// @brief Starts an `INSERT INTO tbl` query.
  InsertInto(const Table &tbl);

  /// @brief Sets the list of columns to insert into.
  /// @throws std::logic_error if the column list was already set, or if the
  /// count does not match an already-set @ref Values row's arity.
  /// @note Only the count is checked; `cols` are not checked to actually
  /// name columns of `tbl`.
  InsertInto Columns(std::initializer_list<Expr> cols) &&;

  /// @brief Sets a single row of values to insert, matching @ref Columns by
  /// position.
  /// @throws std::logic_error if row values were already set (via this or
  /// @ref Rows), or if the count does not match an already-set @ref Columns
  /// list.
  /// @note Only the count is checked; each value's SQL type is not checked
  /// against the corresponding column's declared type.
  InsertInto Values(std::initializer_list<Expr> vals) &&;

  /// @brief Sets multiple rows of values to insert, e.g. `Rows({{1, 2},
  /// {3, 4}})` for `VALUES (1, 2), (3, 4)`.
  /// @throws std::logic_error if row values were already set (via this or
  /// @ref Values), or if any row's arity does not match an already-set
  /// @ref Columns list.
  /// @note Only each row's count is checked, not that every row has the
  /// same arity as every other row, nor value types against columns.
  /// @note Not an overload of @ref Values — for a single-column table,
  /// `{{v}}` can list-initialize either a lone `Expr` or a one-element
  /// `std::initializer_list<Expr>`, so overloading would make that call
  /// ambiguous.
  InsertInto Rows(std::initializer_list<std::initializer_list<Expr>> rows) &&;

  /// @brief Sets the RETURNING clause to a single entry.
  /// @throws std::logic_error if the RETURNING clause was already set.
  InsertInto Returning(SelectItem item) &&;

  /// @brief Sets the RETURNING clause to a comma-separated list of entries.
  /// @throws std::logic_error if the RETURNING clause was already set.
  InsertInto Returning(std::initializer_list<SelectItem> items) &&;

  /// @brief `ON CONFLICT DO NOTHING`, matching any conflict.
  InsertInto OnConflictDoNothing() &&;

  /// @brief `ON CONFLICT (target_cols) DO NOTHING`.
  /// @note Does not check that `target_cols` actually names a unique
  /// index/constraint on `tbl`.
  InsertInto OnConflictDoNothing(std::initializer_list<Expr> target_cols) &&;

  /// @brief `ON CONFLICT (target_cols) DO UPDATE SET column = value, ...`.
  /// Reference the row that triggered the conflict via
  /// `Expr::FromRaw("EXCLUDED.column")`.
  /// @throws std::invalid_argument if `assignments` is empty.
  /// @note Does not check that `target_cols` actually names a unique
  /// index/constraint on `tbl`, nor that `assignments`' columns belong to
  /// `tbl`.
  InsertInto OnConflictDoUpdate(
      std::initializer_list<Expr> target_cols,
      std::initializer_list<std::pair<Expr, Expr>> assignments) &&;

  /// @throws std::logic_error if no columns or no rows were set, or if any
  /// row's arity does not match the columns' count.
  std::string ToString() const;

private:
  std::string into_;
  std::vector<std::string> columns_;
  std::vector<std::vector<std::string>> rows_;
  std::string returning_;
  std::string on_conflict_;
};

} // namespace iron_query
