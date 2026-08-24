#pragma once

#include <iron_query/virtual_table.hpp>
#include <string>

namespace iron_query {

/// @brief Table with name
class [[nodiscard]] Table /* not final! */ : public VirtualTable {
public:
  /// @brief Wraps a trusted, developer-written table name/reference. Never
  /// pass untrusted/dynamic data here — use @ref Expr::Ident to escape a
  /// dynamically-sourced name and pass it as an alias/expression instead.
  static Table FromRaw(std::string name);

  std::string ToString() const override;

  /// @brief Returns the name as-is: a bare table name never needs brackets.
  std::string ToStringBracketed() const override;

  /// @brief Returns the name as-is; this also covers the aliased subqueries
  /// and joins that @ref VirtualTable::As turns into a Table.
  std::string ToStringAsFromItem() const override;

protected:
  Table(std::string name);

private:
  std::string name_;
};

} // namespace iron_query
