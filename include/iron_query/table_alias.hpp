#pragma once

#include <iron_query/column.hpp>
#include <string>
#include <string_view>

namespace iron_query {

/// @brief A table alias, usable to qualify column references as
/// `alias.column`.
class [[nodiscard]] TableAlias final {
public:
  /// @brief Wraps `alias`, which must be a valid (optionally dotted) SQL
  /// identifier.
  /// @throws std::invalid_argument if `alias` is not a valid identifier.
  static TableAlias From(std::string_view alias);

  /// @brief Qualifies a column name as `alias.column`.
  std::string Dot(const std::string &column) const;

  /// @brief Qualifies a column as `alias.column.name`.
  std::string Dot(const Column &column) const;

private:
  TableAlias(std::string_view alias);

  std::string alias_;
};

} // namespace iron_query
