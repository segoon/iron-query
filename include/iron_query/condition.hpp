#pragma once

#include <iron_query/expr.hpp>
#include <iron_query/operator_precedence.hpp>
#include <memory>
#include <string>

namespace iron_query {

namespace impl {
struct Node;
} // namespace impl

/// @brief A boolean-valued SQL predicate, e.g. the result of a comparison,
/// `LIKE`/`IN`/`BETWEEN`/`IS [NOT] NULL`, `EXISTS`, or a logical combination
/// of these. Kept distinct from @ref Expr so that predicate-only positions
/// (`WHERE`, `HAVING`, `ON`, `WHEN`) cannot silently accept a non-boolean
/// expression, e.g. `Where(age)` or `Where(age + 1)`.
///
/// A boolean-valued column or raw expression can be turned into a Condition
/// explicitly via @ref Expr::IsTrue / @ref Expr::IsFalse / @ref
/// Expr::IsNotNull, or via @ref Condition::FromRaw for a trusted raw
/// fragment.
/// @ingroup expressions
class [[nodiscard]] Condition final {
public:
  /// @brief `this AND other`.
  Condition operator&&(const Condition &other) const;

  /// @brief `this OR other`.
  Condition operator||(const Condition &other) const;

  /// @brief `NOT this`.
  Condition operator!() const;

  /// @brief Wraps a trusted, developer-written SQL predicate verbatim. Never
  /// pass untrusted/dynamic data here.
  static Condition FromRaw(std::string s);

  /// @brief Renders the condition as SQL text, parenthesizing it if its
  /// top-level operator does not bind at least as tightly as `precedence`.
  std::string Extract(OperatorPrecedence precedence) const;

  /// @brief Renders the condition as a standalone, unparenthesized SQL
  /// fragment.
  std::string ToString() const;

  /// @brief Converts this predicate into a value expression, e.g. for
  /// embedding it where a boolean-valued Expr is expected. Only available on
  /// rvalues.
  operator Expr() &&;

private:
  friend class Expr;
  friend Condition operator<(const Expr &a, const Expr &b);
  friend Condition operator<=(const Expr &a, const Expr &b);
  friend Condition operator>(const Expr &a, const Expr &b);
  friend Condition operator>=(const Expr &a, const Expr &b);
  friend Condition operator==(const Expr &a, const Expr &b);
  friend Condition operator!=(const Expr &a, const Expr &b);

  Condition(std::string s, OperatorPrecedence precedence);
  explicit Condition(std::shared_ptr<const impl::Node> node);

  std::shared_ptr<const impl::Node> node_;
};

} // namespace iron_query
