#pragma once

#include <iron_query/expr.hpp>
#include <iron_query/select_item.hpp>
#include <string>
#include <string_view>

namespace iron_query {

// userver (???) PostgreSQL part:

/// @brief SQL table column
struct [[nodiscard]] Column final {
  /// Column name
  std::string name;
  /// Type as defined in current SQL dialect
  /// @note This field is metadata only: IQ never consults it to validate
  /// that a compared/assigned Expr matches the column's declared type (see
  /// @ref Expr's class-level @note).
  std::string type;
  /// Whether column value can be NULL
  bool is_nullable{false};

  /// @brief Converts this column reference into an Expr wrapping its name.
  operator Expr() const;

  /// @brief Explicitly converts this column reference into an Expr wrapping
  /// its name, for calling Expr members Column has no dedicated overload
  /// for (e.g. `col.ToExpr().IsNull()`, `col.ToExpr().Between(a, b)`).
  Expr ToExpr() const;

  /// @brief Names this column in a SELECT list: `this AS alias`.
  /// @throws std::invalid_argument if `alias` is not a valid identifier.
  SelectItem As(std::string_view alias) const;
};

} // namespace iron_query
