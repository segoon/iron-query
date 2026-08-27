#pragma once

#include <initializer_list>
#include <iron_query/operator_precedence.hpp>
#include <string>
#include <string_view>
#include <type_traits>

namespace iron_query {

class VirtualTable;
class Condition;
class Collation;
class SelectItem;

namespace impl {

/// @brief Integer types that map onto a SQL integer literal. `bool` and the
/// character types are excluded: they have their own SQL spelling (`TRUE` /
/// a quoted literal), so rendering them as a number would be a silent
/// mistranslation.
template <typename T>
inline constexpr bool kIsSqlInteger =
    std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool> &&
    !std::is_same_v<std::remove_cv_t<T>, char> &&
    !std::is_same_v<std::remove_cv_t<T>, signed char> &&
    !std::is_same_v<std::remove_cv_t<T>, unsigned char> &&
    !std::is_same_v<std::remove_cv_t<T>, wchar_t> &&
    !std::is_same_v<std::remove_cv_t<T>, char16_t> &&
    !std::is_same_v<std::remove_cv_t<T>, char32_t>;

} // namespace impl

/// @brief Arbitrary SQL expression
/// @note The comparison (`<`, `<=`, `>`, `>=`, `==`, `!=`), arithmetic (`+`,
/// `-`, `*`, `/`, `%`, `^`), and index (`operator[]`) operators below do not
/// check that their two operands are SQL-compatible types; e.g. comparing a
/// numeric expression to a text literal renders valid, meaningless SQL.
class [[nodiscard]] Expr final {
public:
  /// @brief Wraps an integer literal of any width and signedness.
  template <typename T, std::enable_if_t<impl::kIsSqlInteger<T>, int> = 0>
  Expr(T value)
      : Expr(FromInteger(
            static_cast<std::conditional_t<std::is_signed_v<T>, long long,
                                           unsigned long long>>(value))) {}

  /// @brief Wraps a floating-point literal, rendered with enough digits to
  /// round-trip back to the same value.
  /// @throws std::invalid_argument if `value` is NaN or infinite: SQL spells
  /// those as typed literals (`'NaN'::float8`), so use @ref FromRaw instead.
  template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
  Expr(T value) : Expr(FromDouble(static_cast<double>(value))) {}

  /// @brief Deleted: a bool would render as a number. Use @ref Bool.
  Expr(bool) = delete;

  /// @brief Deleted: a character would render as its numeric code. Use @ref
  /// Literal for a one-character string.
  Expr(char) = delete;

  /// @brief Deleted: a string literal would decay to `bool`. Use @ref Literal
  /// for a value or @ref FromRaw for a trusted SQL fragment.
  Expr(const char *) = delete;

  /// @brief Wraps a trusted, developer-written SQL fragment verbatim,
  /// together with the precedence of its top-level operator, so that @ref
  /// Extract can bracket it correctly when it is embedded into a
  /// higher-precedence expression. Never pass untrusted/dynamic data here.
  static Expr
  FromRaw(std::string s,
          OperatorPrecedence precedence = OperatorPrecedence::kSymbol);

  /// @brief Builds a properly escaped and quoted SQL string literal out of an
  /// arbitrary (possibly untrusted) value. Use this instead of Expr(string)
  /// whenever the content is not a trusted, developer-written SQL fragment.
  static Expr Literal(const std::string &value);

  /// @brief Builds a properly escaped and quoted SQL identifier out of an
  /// arbitrary (possibly untrusted/dynamic) name. Use this instead of
  /// Expr(string) whenever a table/column name is not a trusted,
  /// developer-written literal.
  static Expr Ident(const std::string &name);

  /// @brief The SQL NULL literal. Note that comparing with it is never true:
  /// use @ref IsNull / @ref IsNotNull to test for it.
  static Expr Null();

  /// @brief The SQL boolean literal `TRUE` or `FALSE`.
  static Expr Bool(bool value);

  /// @brief Builds a function call expression, e.g. Expr::Call("COALESCE",
  /// {a, b}) -> "COALESCE(a, b)". `name` must be a valid (optionally dotted,
  /// e.g. "pg_catalog.now") SQL identifier.
  /// @throws std::invalid_argument if `name` is not a valid identifier.
  /// @note Does not check `args`' count or types against the named SQL
  /// function's actual signature.
  static Expr Call(const std::string &name, std::initializer_list<Expr> args);

  /// @brief COALESCE(args...): the first non-NULL argument.
  /// @throws std::invalid_argument if `args` is empty.
  static Expr Coalesce(std::initializer_list<Expr> args);

  /// @brief NULLIF(a, b): NULL if `a` equals `b`, else `a`.
  static Expr NullIf(const Expr &a, const Expr &b);

  /// @brief GREATEST(args...).
  /// @throws std::invalid_argument if `args` is empty.
  /// @note Unlike @ref Max, this is not an aggregate: it picks the greatest
  /// among its arguments, not among rows.
  static Expr Greatest(std::initializer_list<Expr> args);

  /// @brief LEAST(args...).
  /// @throws std::invalid_argument if `args` is empty.
  /// @note Unlike @ref Min, this is not an aggregate: it picks the least
  /// among its arguments, not among rows.
  static Expr Least(std::initializer_list<Expr> args);

  /// @brief EXISTS (subquery).
  static Condition Exists(const VirtualTable &subquery);

  /// @brief NOT EXISTS (subquery).
  static Condition NotExists(const VirtualTable &subquery);

  /// @brief COUNT(arg).
  static Expr Count(const Expr &arg);

  /// @brief COUNT(*), since "*" is not a valid Expr argument.
  static Expr CountAll();

  /// @brief COUNT(DISTINCT arg).
  static Expr CountDistinct(const Expr &arg);

  /// @brief SUM(arg).
  /// @note Does not check that `arg` is numeric.
  static Expr Sum(const Expr &arg);

  /// @brief AVG(arg).
  /// @note Does not check that `arg` is numeric.
  static Expr Avg(const Expr &arg);

  /// @brief MIN(arg).
  static Expr Min(const Expr &arg);

  /// @brief MAX(arg).
  static Expr Max(const Expr &arg);

  /// @brief Member/field access: `this.other`.
  Expr Dot(const Expr &other) const;

  /// @brief SQL type cast: `CAST (this AS type)`. `type` is a trusted,
  /// developer-written SQL fragment inserted verbatim; never pass
  /// untrusted/dynamic data here.
  /// @note Does not check that `type` is a real SQL type or that casting
  /// `this` to it is valid.
  Expr CastRaw(const std::string &type) const;

  /// @brief `this COLLATE collation`.
  /// @note Does not check that `this` is text-typed; see also @ref
  /// Collation::FromRaw.
  Expr Collate(const Collation &collation) const;

  /// @brief Array/index access: `this[other]`.
  Expr operator[](const Expr &other) const;

  /// @brief Exponentiation: `this ^ other`.
  Expr operator^(const Expr &other) const;

  /// @brief `this BETWEEN a AND b`.
  /// @note Does not check that `a` and `b` share a comparable type with
  /// `this`, nor that `a <= b`.
  Condition Between(const Expr &a, const Expr &b) const;

  /// @brief `this NOT BETWEEN a AND b`.
  /// @note Does not check that `a` and `b` share a comparable type with
  /// `this`, nor that `a <= b`.
  Condition NotBetween(const Expr &a, const Expr &b) const;

  /// @brief `this LIKE a`.
  /// @note Does not check that `this` and `a` are text-typed.
  Condition Like(const Expr &a) const;

  /// @brief `this NOT LIKE a`.
  /// @note Does not check that `this` and `a` are text-typed.
  Condition NotLike(const Expr &a) const;

  /// @brief `this ILIKE a`, a case-insensitive `LIKE`.
  /// @note Does not check that `this` and `a` are text-typed.
  Condition ILike(const Expr &a) const;

  /// @brief `this NOT ILIKE a`.
  /// @note Does not check that `this` and `a` are text-typed.
  Condition NotILike(const Expr &a) const;

  /// @brief `this SIMILAR TO a`.
  /// @note Does not check that `this` and `a` are text-typed.
  Condition SimilarTo(const Expr &a) const;

  /// @brief `this NOT SIMILAR TO a`.
  /// @note Does not check that `this` and `a` are text-typed.
  Condition NotSimilarTo(const Expr &a) const;

  /// @brief `this IN (a, b, c)`.
  /// @throws std::invalid_argument if `values` is empty: SQL has no empty
  /// `IN ()` list.
  /// @note Does not check that all `values` share a common type with `this`
  /// or with each other.
  Condition In(std::initializer_list<Expr> values) const;

  /// @brief `this IN (subquery)`.
  Condition In(const VirtualTable &subquery) const;

  /// @brief `this NOT IN (a, b, c)`.
  /// @throws std::invalid_argument if `values` is empty.
  /// @note Does not check that all `values` share a common type with `this`
  /// or with each other.
  Condition NotIn(std::initializer_list<Expr> values) const;

  /// @brief `this NOT IN (subquery)`.
  Condition NotIn(const VirtualTable &subquery) const;

  /// @brief `this = ANY (array)`. The idiomatic way to test membership in a
  /// list that arrives as a single bind parameter, e.g. `col.EqAny(_1)`.
  /// @note Does not check that `array` is actually array-typed or that its
  /// element type is compatible with `this`.
  Condition EqAny(const Expr &array) const;

  /// @brief `this = ANY (subquery)`.
  Condition EqAny(const VirtualTable &subquery) const;

  /// @brief `this <> ALL (array)`, the negation of @ref EqAny.
  /// @note Does not check that `array` is actually array-typed or that its
  /// element type is compatible with `this`.
  Condition NeAll(const Expr &array) const;

  /// @brief `this <> ALL (subquery)`.
  Condition NeAll(const VirtualTable &subquery) const;

  /// @brief `this IS TRUE`.
  Condition IsTrue() const;

  /// @brief `this IS FALSE`.
  Condition IsFalse() const;

  /// @brief `this IS NULL`.
  Condition IsNull() const;

  /// @brief `this IS NOT NULL`.
  Condition IsNotNull() const;

  /// @brief `this IS DISTINCT FROM other`, the NULL-safe inequality
  /// comparison: unlike `!=`, it is never NULL, treating two NULLs as equal.
  Condition IsDistinctFrom(const Expr &other) const;

  /// @brief `this IS NOT DISTINCT FROM other`, the NULL-safe equality
  /// comparison.
  Condition IsNotDistinctFrom(const Expr &other) const;

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

  /// @brief `this + other`.
  Expr operator+(const Expr &other) const;

  /// @brief `this - other`.
  Expr operator-(const Expr &other) const;

  /// @brief `this * other`.
  Expr operator*(const Expr &other) const;

  /// @brief `this / other`.
  Expr operator/(const Expr &other) const;

  /// @brief `this % other`.
  Expr operator%(const Expr &other) const;

  /// @brief Names this expression in a SELECT list: `this AS name`. `name`
  /// must be a plain (undotted) SQL identifier.
  /// @throws std::invalid_argument if `name` is not a valid identifier.
  SelectItem As(std::string_view name) const;

  /// @brief Renders the expression as SQL text, parenthesizing it if its
  /// top-level operator does not bind at least as tightly as `precedence`.
  /// @param precedence The precedence context this expression is being
  /// embedded into.
  /// @return The (possibly parenthesized) SQL fragment.
  std::string Extract(OperatorPrecedence precedence) const;

  /// @brief Renders the expression as a standalone, unparenthesized SQL
  /// fragment.
  std::string ToString() const;

private:
  friend class Condition;

  Expr(std::string s);
  Expr(std::string expr, OperatorPrecedence precedence);

  static Expr FromInteger(long long value);
  static Expr FromInteger(unsigned long long value);
  static Expr FromDouble(double value);

  std::string expr_;
  OperatorPrecedence precedence_;
};

} // namespace iron_query
