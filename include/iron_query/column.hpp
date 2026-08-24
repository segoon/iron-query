#pragma once

#include <iron_query/condition.hpp>
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
  std::string type;
  /// Whether column value can be NULL
  bool is_nullable{false};

  /// @brief Converts this column reference into an Expr wrapping its name.
  operator Expr() const;

  /// @brief Names this column in a SELECT list: `this AS alias`.
  /// @throws std::invalid_argument if `alias` is not a valid identifier.
  SelectItem As(std::string_view alias) const;

  /// @brief `this < other`.
  Condition operator<(const Expr &other) const;
  /// @brief `this <= other`.
  Condition operator<=(const Expr &other) const;
  /// @brief `this > other`.
  Condition operator>(const Expr &other) const;
  /// @brief `this >= other`.
  Condition operator>=(const Expr &other) const;
  /// @brief `this = other`.
  Condition operator==(const Expr &other) const;
  /// @brief `this != other`.
  Condition operator!=(const Expr &other) const;
};

} // namespace iron_query
