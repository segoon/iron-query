#pragma once

#include <iron_query/virtual_table.hpp>
#include <string>

namespace iron_query {

/// @brief Finalized WITH ... query, usable anywhere a VirtualTable is
/// (subquery, FROM source, etc.)
class [[nodiscard]] WithQuery final : public VirtualTable {
public:
  WithQuery(std::string ctes, std::string main);

  std::string ToString() const override;

private:
  std::string ctes_;
  std::string main_;
};

/// @brief Transitional representation for a WITH clause being built up
class [[nodiscard]] WithBuilder final {
public:
  /// @brief Adds another CTE: `, name AS (query)`. `name` must be a valid
  /// (optionally dotted) SQL identifier.
  /// @throws std::invalid_argument if `name` is not a valid identifier.
  WithBuilder With(std::string name, const VirtualTable &query) &&;

  /// @brief Finalizes the WITH clause with the main query that follows it.
  WithQuery Main(const VirtualTable &query) &&;

private:
  /// @brief Starts a WITH clause with a single CTE: `name AS (query)`.
  WithBuilder(std::string name, const VirtualTable &query);

  std::string ctes_;

  friend WithBuilder With(std::string name, const VirtualTable &query);
};

/// @brief Handy fabric for @ref WithBuilder. `name` must be a valid
/// (optionally dotted) SQL identifier.
/// @throws std::invalid_argument if `name` is not a valid identifier.
WithBuilder With(std::string name, const VirtualTable &query);

} // namespace iron_query
