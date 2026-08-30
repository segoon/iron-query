#pragma once

#include <initializer_list>
#include <iron_query/condition.hpp>
#include <iron_query/select_item.hpp>
#include <iron_query/table.hpp>
#include <iron_query/virtual_table.hpp>
#include <string>

namespace iron_query {

/// @brief Transitional representation for DELETE FROM query
/// @see https://www.postgresql.org/docs/current/sql-delete.html
/// @ingroup statements
class [[nodiscard]] DeleteFrom final : public VirtualTable {
public:
  /// @brief Starts a `DELETE FROM tbl` query.
  DeleteFrom(const Table &tbl);

  /// @brief Sets the USING clause, so WHERE expressions may reference
  /// columns of `tbl`. `tbl` may be a table, a join, or anything @ref
  /// VirtualTable::As has aliased.
  /// @throws LogicError if `tbl` is a subquery without an alias, or if
  /// the USING clause was already set.
  DeleteFrom Using(const VirtualTable &tbl) &&;

  /// @brief Sets the WHERE clause.
  /// @throws LogicError if the WHERE clause was already set.
  DeleteFrom Where(Condition exp) &&;

  /// @brief Sets the RETURNING clause to a single entry.
  /// @throws LogicError if the RETURNING clause was already set.
  DeleteFrom Returning(SelectItem item) &&;

  /// @brief Sets the RETURNING clause to a comma-separated list of entries.
  /// @throws LogicError if the RETURNING clause was already set.
  DeleteFrom Returning(std::initializer_list<SelectItem> items) &&;

  /// @throws LogicError if the FROM clause was not set.
  std::string ToString() const override;

private:
  std::string from_;
  std::string using_;
  std::string where_;
  std::string returning_;
};

} // namespace iron_query
