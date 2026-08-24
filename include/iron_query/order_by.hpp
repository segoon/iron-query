#pragma once

#include <iron_query/expr.hpp>

namespace iron_query {

/// @brief Sort direction for a single ORDER BY term. See @ref OrderByTerm.
enum class SortDirection {
  kAscending,
  kDescending,
};

/// @brief A single ORDER BY term: an expression plus its sort direction.
/// Implicitly constructible from just an @ref Expr for the common ascending
/// case, e.g. `OrderBy({col1, {col2, SortDirection::kDescending}})`.
struct [[nodiscard]] OrderByTerm final {
  /// @brief Wraps `expr` with the given sort `direction` (ascending by
  /// default).
  OrderByTerm(Expr expr, SortDirection direction = SortDirection::kAscending);

  Expr expr;
  SortDirection direction;
};

} // namespace iron_query
