#pragma once

#include <initializer_list>
#include <iron_query/condition.hpp>
#include <iron_query/order_by.hpp>
#include <iron_query/select_item.hpp>
#include <iron_query/virtual_table.hpp>
#include <optional>
#include <string>
#include <vector>

namespace iron_query {

/// @brief Transitional representation for SELECT query
/// @see https://www.postgresql.org/docs/current/sql-select.html
/// @ingroup statements
class [[nodiscard]] SelectExpr final : public VirtualTable {
public:
  /// @brief Starts a `SELECT ... FROM tbl` query. `tbl` may be a table, a
  /// join, or anything @ref VirtualTable::As has aliased.
  /// @throws LogicError if `tbl` is a subquery without an alias.
  SelectExpr(const VirtualTable &tbl);

  /// @brief Adds `DISTINCT` to the SELECT list, eliminating duplicate rows.
  SelectExpr Distinct() &&;

  /// @brief Sets DISTINCT ON to a single expression. Mutually exclusive with
  /// @ref Distinct.
  /// @throws LogicError if DISTINCT ON was already set.
  SelectExpr DistinctOn(Expr exp) &&;

  /// @brief Sets DISTINCT ON to a comma-separated list of expressions.
  /// Mutually exclusive with @ref Distinct.
  /// @note Does not check that the leftmost `ORDER BY` expressions match
  /// these, which PostgreSQL requires.
  /// @throws LogicError if DISTINCT ON was already set.
  SelectExpr DistinctOn(std::initializer_list<Expr> exps) &&;

  /// @brief Sets the SELECT list to a single entry.
  /// @throws LogicError if the SELECT list was already set.
  SelectExpr Select(SelectItem item) &&;

  /// @brief Sets the SELECT list to a comma-separated list of entries.
  /// @throws LogicError if the SELECT list was already set.
  SelectExpr Select(std::initializer_list<SelectItem> items) &&;

  /// @brief Sets the WHERE clause.
  /// @throws LogicError if the WHERE clause was already set.
  SelectExpr Where(Condition exp) &&;

  /// @brief Sets the ORDER BY clause to a single term.
  /// @throws LogicError if the ORDER BY clause was already set.
  SelectExpr OrderBy(OrderByTerm term) &&;

  /// @brief Sets the ORDER BY clause to a comma-separated list of terms.
  /// @throws LogicError if the ORDER BY clause was already set.
  SelectExpr OrderBy(std::initializer_list<OrderByTerm> terms) &&;

  /// @brief Sets the GROUP BY clause to a single expression.
  /// @note Does not check that non-aggregated @ref Select items are covered
  /// by the GROUP BY expressions.
  /// @throws LogicError if the GROUP BY clause was already set.
  SelectExpr GroupBy(Expr exp) &&;

  /// @brief Sets the GROUP BY clause to a comma-separated list of
  /// expressions.
  /// @note Does not check that non-aggregated @ref Select items are covered
  /// by the GROUP BY expressions.
  /// @throws LogicError if the GROUP BY clause was already set.
  SelectExpr GroupBy(std::initializer_list<Expr> exps) &&;

  /// @brief Sets the HAVING clause.
  /// @note Does not check that `exp` references only grouped columns or
  /// aggregates.
  /// @throws LogicError if the HAVING clause was already set.
  SelectExpr Having(Condition exp) &&;

  /// @brief Sets the LIMIT clause.
  /// @throws LogicError if the LIMIT clause was already set.
  SelectExpr Limit(int limit) &&;

  /// @brief Sets the OFFSET clause.
  /// @throws LogicError if the OFFSET clause was already set.
  SelectExpr Offset(int offset) &&;

  /// @throws LogicError if the SELECT or FROM clause was not set, or
  /// if both @ref Distinct and @ref DistinctOn were set.
  std::string ToString() const override;

  /// @brief Renders the query with each clause on its own line, indented,
  /// e.g.:
  /// ```
  /// SELECT
  ///     a,
  ///     b
  /// FROM
  ///     users
  /// WHERE
  ///     a > b
  /// ```
  /// @throws LogicError if the SELECT or FROM clause was not set.
  std::string ToStringFormatted() const override;

private:
  void EnsureValid() const;

  bool distinct_{false};
  std::optional<std::vector<Expr>> distinct_on_;
  std::string from_;
  std::optional<std::vector<SelectItem>> select_;
  std::optional<Condition> where_;
  std::optional<std::vector<Expr>> group_by_;
  std::optional<Condition> having_;
  std::optional<std::vector<OrderByTerm>> order_by_;
  std::optional<int> limit_;
  std::optional<int> offset_;
};

/// @brief Handy fabric for @ref SelectExpr
/// @ingroup statements
SelectExpr From(const VirtualTable &tbl);

} // namespace iron_query
