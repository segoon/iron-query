#pragma once

#include <iron_query/condition.hpp>
#include <iron_query/expr.hpp>
#include <string>

namespace iron_query {

/// @brief Transitional representation for CASE WHEN ... END
class [[nodiscard]] CaseBuilder final {
public:
  /// @brief Starts an empty CASE expression; call @ref When to add branches.
  CaseBuilder() = default;

  /// @brief Begins a new `WHEN cond` branch.
  /// @throws std::logic_error if called twice in a row without a matching
  /// @ref Then in between.
  CaseBuilder When(Condition cond) &&;

  /// @brief Completes the pending branch as `WHEN cond THEN result`.
  /// @throws std::logic_error if not preceded by @ref When.
  /// @note Does not check that all `Then`/`Else` branches of the same CASE
  /// share a compatible result type.
  CaseBuilder Then(Expr result) &&;

  /// @brief Sets the `ELSE result` fallback branch.
  /// @note Does not check that all `Then`/`Else` branches of the same CASE
  /// share a compatible result type.
  CaseBuilder Else(Expr result) &&;

  /// @brief Finalizes the expression as `CASE ... [ELSE ...] END`.
  /// @throws std::logic_error if no `WHEN`/`THEN` pair was added.
  Expr End() const;

private:
  std::string whens_;
  std::string pending_when_;
  std::string else_;
  bool has_pending_when_{false};
};

/// @brief Handy fabric for @ref CaseBuilder
CaseBuilder Case();

} // namespace iron_query
