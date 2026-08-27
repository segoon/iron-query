#pragma once

#include <iron_query/expr.hpp>

namespace iron_query {

/// @brief Sort direction for a single ORDER BY term. See @ref OrderByTerm.
enum class SortDirection {
  kAscending,
  kDescending,
};

/// @brief Placement of NULLs within an ORDER BY term. See @ref OrderByTerm.
enum class NullsOrder {
  /// @brief No explicit `NULLS FIRST/LAST`; the server's default applies
  /// (PostgreSQL: last for ascending, first for descending).
  kDefault,
  kFirst,
  kLast,
};

/// @brief A single ORDER BY term: an expression plus its sort direction and
/// NULL placement. Implicitly constructible from just an @ref Expr for the
/// common ascending case, e.g. `OrderBy({col1, {col2,
/// SortDirection::kDescending}})`.
struct [[nodiscard]] OrderByTerm final {
  /// @brief Wraps `expr` with the given sort `direction` (ascending by
  /// default) and `nulls_order` (server default by default).
  OrderByTerm(Expr expr, SortDirection direction = SortDirection::kAscending,
              NullsOrder nulls_order = NullsOrder::kDefault);

  Expr expr;
  SortDirection direction;
  NullsOrder nulls_order;
};

} // namespace iron_query
