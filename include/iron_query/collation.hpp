#pragma once

#include <string>

namespace iron_query {

/// @brief A collation name for use with @ref Expr::Collate.
class [[nodiscard]] Collation final {
public:
  /// @brief Wraps a trusted, developer-written collation name verbatim.
  /// Never pass untrusted/dynamic data here.
  static Collation FromRaw(std::string name);

  /// @brief The database's default collation.
  static Collation Default();
  /// @brief The `C` collation: byte-order comparison.
  static Collation C();
  /// @brief The `POSIX` collation, identical to `C` on PostgreSQL.
  static Collation Posix();

  /// @brief Renders the collation name as SQL text.
  std::string ToString() const;

private:
  Collation(std::string name);

  std::string name_;
};

} // namespace iron_query
