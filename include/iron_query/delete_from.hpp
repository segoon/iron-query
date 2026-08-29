#pragma once

#include <initializer_list>
#include <iron_query/condition.hpp>
#include <iron_query/select_item.hpp>
#include <iron_query/table.hpp>
#include <iron_query/virtual_table.hpp>
#include <string>

namespace iron_query {

/// @brief Transitional representation for DELETE FROM query
class [[nodiscard]] DeleteFrom final : public VirtualTable {
public:
  /// @brief Starts a `DELETE FROM tbl` query.
  DeleteFrom(const Table &tbl);

  /// @brief Sets the USING clause, so WHERE expressions may reference
  /// columns of `tbl`. `tbl` may be a table, a join, or anything @ref
  /// VirtualTable::As has aliased.
  /// @throws std::logic_error if `tbl` is a subquery without an alias.
  DeleteFrom Using(const VirtualTable &tbl) &&;

  /// @brief Sets the WHERE clause.
  DeleteFrom Where(Condition exp) &&;

  /// @brief Sets the RETURNING clause to a single entry, replacing any
  /// previously set one.
  DeleteFrom Returning(SelectItem item) &&;

  /// @brief Sets the RETURNING clause to a comma-separated list of entries,
  /// replacing any previously set one.
  DeleteFrom Returning(std::initializer_list<SelectItem> items) &&;

  /// @throws std::logic_error if the FROM clause was not set.
  std::string ToString() const override;

private:
  std::string from_;
  std::string using_;
  std::string where_;
  std::string returning_;
};

} // namespace iron_query
