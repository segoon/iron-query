#pragma once

#include <iron_query/condition.hpp>
#include <iron_query/table.hpp>
#include <iron_query/virtual_table.hpp>
#include <string>

namespace iron_query {

/// @brief Transitional representation for DELETE FROM query
class [[nodiscard]] DeleteFrom final : public VirtualTable {
public:
  /// @brief Starts a `DELETE FROM tbl` query.
  DeleteFrom(const Table &tbl);

  /// @brief Sets the WHERE clause.
  DeleteFrom Where(Condition exp) &&;

  /// @throws std::logic_error if the FROM clause was not set.
  std::string ToString() const override;

private:
  std::string from_;
  std::string where_;
};

} // namespace iron_query
