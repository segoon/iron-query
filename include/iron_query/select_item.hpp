#pragma once

#include <iron_query/expr.hpp>
#include <string>
#include <type_traits>
#include <utility>

namespace iron_query {

/// @brief A single entry of a SELECT list: an expression, optionally renamed
/// with @ref Expr::As. Kept distinct from @ref Expr because `x AS y` is legal
/// nowhere else, so `Where(x.As("y"))` or `Expr::Call("ABS", {x.As("y")})` do
/// not compile.
/// @ingroup expressions
class [[nodiscard]] SelectItem final {
public:
  /// @brief Implicitly wraps anything an @ref Expr can be built from, e.g. an
  /// Expr, a @ref Column, or an rvalue @ref Condition.
  template <typename T,
            std::enable_if_t<std::is_convertible_v<T, Expr>, int> = 0>
  SelectItem(T &&value) : SelectItem(Expr(std::forward<T>(value)).ToString()) {}

  /// @brief Renders the entry as SQL text.
  std::string ToString() const;

private:
  friend class Expr;

  explicit SelectItem(std::string s);

  std::string s_;
};

} // namespace iron_query
