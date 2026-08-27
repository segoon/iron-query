#pragma once

#include <iron_query/virtual_table.hpp>
#include <string>
#include <string_view>

namespace iron_query {

/// @brief Kind of SQL set operation, e.g. @ref Union or @ref Except.
struct [[nodiscard]] SetOpKind {
  /// @brief The SQL keyword(s) for this set operation, e.g. "UNION".
  virtual std::string_view ToString() const = 0;
};

/// @brief UNION.
struct [[nodiscard]] Union final : SetOpKind {
  std::string_view ToString() const override;
};
/// @brief UNION ALL.
struct [[nodiscard]] UnionAll final : SetOpKind {
  std::string_view ToString() const override;
};
/// @brief INTERSECT.
struct [[nodiscard]] Intersect final : SetOpKind {
  std::string_view ToString() const override;
};
/// @brief EXCEPT.
struct [[nodiscard]] Except final : SetOpKind {
  std::string_view ToString() const override;
};

/// @brief Transitional representation for UNION/UNION ALL/INTERSECT/EXCEPT
class [[nodiscard]] SetOp final : public VirtualTable {
public:
  /// @brief Builds `a kind b`.
  /// @note Does not check that `a` and `b` produce the same number of
  /// columns or compatible column types.
  SetOp(const VirtualTable &a, const VirtualTable &b, const SetOpKind &kind);

  std::string ToString() const override;

private:
  std::string a_, b_;
  std::string kind_;
};

} // namespace iron_query
