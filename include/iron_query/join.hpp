#pragma once

#include <initializer_list>
#include <iron_query/condition.hpp>
#include <iron_query/virtual_table.hpp>
#include <string>
#include <string_view>

namespace iron_query {

/// @brief Kind of SQL JOIN, e.g. @ref Inner or @ref LeftOuter.
struct [[nodiscard]] JoinKind {
  /// @brief The SQL keyword(s) for this join kind, e.g. "INNER".
  virtual std::string_view ToString() const = 0;

  /// @brief Whether this join kind is a NATURAL join, which forbids an
  /// explicit ON/USING qualification.
  virtual bool IsNatural() const { return false; }
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
/// @brief NATURAL INNER JOIN: joins on all identically named columns.
struct [[nodiscard]] NaturalInner final : JoinKind {
  std::string_view ToString() const override;
  bool IsNatural() const override { return true; }
};
/// @brief NATURAL LEFT OUTER JOIN.
struct [[nodiscard]] NaturalLeftOuter final : JoinKind {
  std::string_view ToString() const override;
  bool IsNatural() const override { return true; }
};
/// @brief NATURAL RIGHT OUTER JOIN.
struct [[nodiscard]] NaturalRightOuter final : JoinKind {
  std::string_view ToString() const override;
  bool IsNatural() const override { return true; }
};
/// @brief NATURAL FULL OUTER JOIN.
struct [[nodiscard]] NaturalFullOuter final : JoinKind {
  std::string_view ToString() const override;
  bool IsNatural() const override { return true; }
};

/// @brief Transitional representation for JOIN query
class [[nodiscard]] Join final : public VirtualTable {
public:
  /// @brief Builds `a kind JOIN b`.
  Join(const VirtualTable &a, const VirtualTable &b, const JoinKind &kind);

  /// @brief Sets the ON clause.
  /// @note Does not check that `exp` references only columns of the two
  /// joined tables.
  /// @throws std::logic_error if this is a NATURAL join, if the ON clause
  /// was already set, or if a USING clause was already set.
  Join On(Condition exp) &&;

  /// @brief Sets the USING (cols) clause: joins by equating same-named
  /// columns from each side.
  /// @throws std::logic_error if this is a NATURAL join, if a USING clause
  /// was already set, or if an ON clause was already set.
  /// @throws std::invalid_argument if `columns` is empty.
  Join Using(std::initializer_list<Expr> columns) &&;

  std::string ToString() const override;

  /// @brief Returns the join unbracketed: PostgreSQL only allows parentheses
  /// around a FROM-item join when an alias follows.
  std::string ToStringAsFromItem() const override;

private:
  std::string a_, b_;
  std::string kind_;
  bool natural_;
  std::string on_;
  std::string using_;
};

} // namespace iron_query
