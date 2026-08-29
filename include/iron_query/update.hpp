#pragma once

#include <initializer_list>
#include <iron_query/condition.hpp>
#include <iron_query/expr.hpp>
#include <iron_query/select_item.hpp>
#include <iron_query/table.hpp>
#include <iron_query/virtual_table.hpp>
#include <string>

namespace iron_query {

/// @brief Transitional representation for UPDATE query
class [[nodiscard]] Update final {
public:
  /// @brief Starts an `UPDATE tbl` query.
  Update(const Table &tbl);

  /// @brief Adds a `column = value` assignment to the SET clause.
  /// @note Does not check that `column` names an actual column of `tbl`,
  /// nor that `value`'s type matches that column's declared type.
  Update Set(const Expr &column, const Expr &value) &&;

  /// @brief Sets the FROM clause, so SET/WHERE expressions may reference
  /// columns of `tbl`. `tbl` may be a table, a join, or anything @ref
  /// VirtualTable::As has aliased.
  /// @throws std::logic_error if `tbl` is a subquery without an alias.
  Update From(const VirtualTable &tbl) &&;

  /// @brief Sets the WHERE clause.
  Update Where(Condition exp) &&;

  /// @brief Sets the RETURNING clause to a single entry, replacing any
  /// previously set one.
  Update Returning(SelectItem item) &&;

  /// @brief Sets the RETURNING clause to a comma-separated list of entries,
  /// replacing any previously set one.
  Update Returning(std::initializer_list<SelectItem> items) &&;

  /// @throws std::logic_error if no SET assignment was added.
  std::string ToString() const;

private:
  std::string table_;
  std::string set_;
  std::string from_;
  std::string where_;
  std::string returning_;
};

} // namespace iron_query
