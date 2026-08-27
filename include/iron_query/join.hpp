#pragma once

#include <iron_query/condition.hpp>
#include <iron_query/virtual_table.hpp>
#include <string>
#include <string_view>

namespace iron_query {

/// @brief Kind of SQL JOIN, e.g. @ref Inner or @ref LeftOuter.
struct [[nodiscard]] JoinKind {
  /// @brief The SQL keyword(s) for this join kind, e.g. "INNER".
  virtual std::string_view ToString() const = 0;
};

/// @brief INNER JOIN.
struct [[nodiscard]] Inner final : JoinKind {
  std::string_view ToString() const override;
};
/// @brief CROSS JOIN.
struct [[nodiscard]] Cross final : JoinKind {
  std::string_view ToString() const override;
};
/// @brief LEFT OUTER JOIN.
struct [[nodiscard]] LeftOuter final : JoinKind {
  std::string_view ToString() const override;
};
/// @brief RIGHT OUTER JOIN.
struct [[nodiscard]] RightOuter final : JoinKind {
  std::string_view ToString() const override;
};
/// @brief FULL OUTER JOIN.
struct [[nodiscard]] FullOuter final : JoinKind {
  std::string_view ToString() const override;
};

/// @brief Transitional representation for JOIN query
class [[nodiscard]] Join final : public VirtualTable {
public:
  /// @brief Builds `a kind JOIN b`.
  Join(const VirtualTable &a, const VirtualTable &b, const JoinKind &kind);

  /// @brief Sets the ON clause.
  /// @note Does not check that `exp` references only columns of the two
  /// joined tables.
  Join On(Condition exp) &&;

  std::string ToString() const override;

  /// @brief Returns the join unbracketed: PostgreSQL only allows parentheses
  /// around a FROM-item join when an alias follows.
  std::string ToStringAsFromItem() const override;

private:
  std::string a_, b_;
  std::string kind_;
  std::string on_;
};

} // namespace iron_query
