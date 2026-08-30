#pragma once

#include <iron_query/expr.hpp>
#include <string>
#include <string_view>

namespace iron_query {

class Table;

/// @brief Any table value, including physical table, table result,
/// materialized view, etc.
/// @ingroup schema
class [[nodiscard]] VirtualTable {
public:
  /// @brief Renders this table value as single-line SQL text. See also @ref
  /// ToStringFormatted for a multi-line, indented rendering.
  virtual std::string ToString() const = 0;

  /// @brief Renders this table value as multi-line, indented SQL text for
  /// readability (e.g. logging/debugging a generated query). Defaults to
  /// @ref ToString; overridden by @ref SelectExpr to lay out each clause on
  /// its own line.
  virtual std::string ToStringFormatted() const;

  /// @brief Renders this table value as SQL text, parenthesized so it can be
  /// safely embedded as a subquery. Overridden by @ref Table to skip
  /// bracketing, since a bare table name never needs it.
  virtual std::string ToStringBracketed() const;

  /// @brief Renders this table value as a FROM item. Distinct from @ref
  /// ToStringBracketed because PostgreSQL's `table_ref` grammar accepts a bare
  /// join but parenthesizes one only when an alias follows.
  /// @throws LogicError for table values that PostgreSQL requires an
  /// alias for, i.e. every subquery; call @ref As first.
  virtual std::string ToStringAsFromItem() const;

  /// @brief Aliases this table value as `this AS name`, usable as a FROM
  /// source. `name` must be a valid (optionally dotted) SQL identifier.
  /// @throws InvalidIdentifier if `name` is not a valid identifier.
  Table As(std::string_view name) const;

  /// @brief Converts this table value into a subquery expression, e.g. for
  /// use as a scalar subquery. Only available on rvalues.
  /// @note Does not check that the underlying query yields exactly one
  /// column: a multi-column query converts just as freely as a
  /// single-column one.
  operator Expr() const &&;
};

} // namespace iron_query
