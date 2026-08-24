#pragma once

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace iron_query {

/// @brief Relative binding strength of SQL operators, used by @ref
/// Expr::Extract to decide whether a sub-expression needs to be
/// parenthesized when embedded into a larger expression. From
/// https://www.postgresql.org/docs/current/sql-syntax-lexical.html#SQL-PRECEDENCE
enum class OperatorPrecedence {
  kSymbol,
  kDot,
  kTypecast,
  kIndex,
  kUnaryPlus,
  kCollate,
  kAt,
  kExp,
  kMul,
  kPlus,
  kAnyOther,
  kBetween,
  kCompare,
  kIs,
  kNot,
  kAnd,
  kOr,
  kExtract, // pseudo precedence for "no brackets"
};

class VirtualTable;
class Condition;
class Collation;
class SelectItem;

namespace detail {

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

} // namespace detail

/// @brief Arbitrary SQL expression
class [[nodiscard]] Expr final {
public:
  /// @brief Wraps an integer literal of any width and signedness.
  template <typename T, std::enable_if_t<detail::kIsSqlInteger<T>, int> = 0>
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

  /// @brief Wraps a trusted, developer-written SQL fragment verbatim. Never
  /// pass untrusted/dynamic data here — use @ref Literal or @ref Ident
  /// instead, since this string is inserted into the query unescaped.
  static Expr FromRaw(std::string s);

  /// @brief Wraps a trusted, developer-written SQL fragment verbatim,
  /// together with the precedence of its top-level operator, so that @ref
  /// Extract can bracket it correctly when it is embedded into a
  /// higher-precedence expression. Never pass untrusted/dynamic data here.
  static Expr FromRaw(std::string s, OperatorPrecedence precedence);

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
  static Expr Call(const std::string &name, std::initializer_list<Expr> args);

  /// @brief EXISTS (subquery).
  static Condition Exists(const VirtualTable &subquery);

  /// @brief NOT EXISTS (subquery).
  static Condition NotExists(const VirtualTable &subquery);

  /// @brief COUNT(arg).
  static Expr Count(const Expr &arg);

  /// @brief COUNT(*), since "*" is not a valid Expr argument.
  static Expr CountAll();

  /// @brief SUM(arg).
  static Expr Sum(const Expr &arg);

  /// @brief AVG(arg).
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
  Expr CastRaw(const std::string &type) const;

  /// @brief `this COLLATE collation`.
  Expr Collate(const Collation &collation) const;

  /// @brief Array/index access: `this[other]`.
  Expr operator[](const Expr &other) const;

  /// @brief Exponentiation: `this ^ other`.
  Expr operator^(const Expr &other) const;

  /// @brief `this BETWEEN a AND b`.
  Condition Between(const Expr &a, const Expr &b) const;

  /// @brief `this NOT BETWEEN a AND b`.
  Condition NotBetween(const Expr &a, const Expr &b) const;

  /// @brief `this LIKE a`.
  Condition Like(const Expr &a) const;

  /// @brief `this NOT LIKE a`.
  Condition NotLike(const Expr &a) const;

  /// @brief `this IN (a, b, c)`.
  /// @throws std::invalid_argument if `values` is empty: SQL has no empty
  /// `IN ()` list.
  Condition In(std::initializer_list<Expr> values) const;

  /// @brief `this IN (subquery)`.
  Condition In(const VirtualTable &subquery) const;

  /// @brief `this NOT IN (a, b, c)`.
  /// @throws std::invalid_argument if `values` is empty.
  Condition NotIn(std::initializer_list<Expr> values) const;

  /// @brief `this NOT IN (subquery)`.
  Condition NotIn(const VirtualTable &subquery) const;

  /// @brief `this = ANY (array)`. The idiomatic way to test membership in a
  /// list that arrives as a single bind parameter, e.g. `col.EqAny(_1)`.
  Condition EqAny(const Expr &array) const;

  /// @brief `this = ANY (subquery)`.
  Condition EqAny(const VirtualTable &subquery) const;

  /// @brief `this <> ALL (array)`, the negation of @ref EqAny.
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
  operator Expr() const &&;

private:
  friend class Expr;

  Condition(std::string s, OperatorPrecedence precedence);

  std::string expr_;
  OperatorPrecedence precedence_;
};

/// @brief A single entry of a SELECT list: an expression, optionally renamed
/// with @ref Expr::As. Kept distinct from @ref Expr because `x AS y` is legal
/// nowhere else, so `Where(x.As("y"))` or `Expr::Call("ABS", {x.As("y")})` do
/// not compile.
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

/// @brief Transitional representation for CASE WHEN ... END
class [[nodiscard]] CaseBuilder final {
public:
  /// @brief Starts an empty CASE expression; call @ref When to add branches.
  CaseBuilder() = default;

  /// @brief Begins a new `WHEN cond` branch.
  /// @throws std::logic_error if called twice in a row without a matching
  /// @ref Then in between.
  CaseBuilder When(Condition cond) &&;

  /// @brief Completes the pending branch as `WHEN cond THEN result`.
  /// @throws std::logic_error if not preceded by @ref When.
  CaseBuilder Then(Expr result) &&;

  /// @brief Sets the `ELSE result` fallback branch.
  CaseBuilder Else(Expr result) &&;

  /// @brief Finalizes the expression as `CASE ... [ELSE ...] END`.
  /// @throws std::logic_error if no `WHEN`/`THEN` pair was added.
  Expr End() const;

private:
  std::string whens_;
  std::string pending_when_;
  std::string else_;
  bool has_pending_when_{false};
};

/// @brief Handy fabric for @ref CaseBuilder
CaseBuilder Case();

/// @brief Sort direction for a single ORDER BY term. See @ref OrderByTerm.
enum class SortDirection {
  kAscending,
  kDescending,
};

/// @brief A single ORDER BY term: an expression plus its sort direction.
/// Implicitly constructible from just an @ref Expr for the common ascending
/// case, e.g. `OrderBy({col1, {col2, SortDirection::kDescending}})`.
struct [[nodiscard]] OrderByTerm final {
  /// @brief Wraps `expr` with the given sort `direction` (ascending by
  /// default).
  OrderByTerm(Expr expr, SortDirection direction = SortDirection::kAscending);

  Expr expr;
  SortDirection direction;
};

/// @brief A synonym for $1
extern const Expr _1;
extern const Expr _2;
extern const Expr _3;
extern const Expr _4;
extern const Expr _5;
extern const Expr _6;
extern const Expr _7;
extern const Expr _8;
extern const Expr _9;
extern const Expr _10;

class Table;

/// @brief Any table value, including physical table, table result,
/// materialized view, etc.
class [[nodiscard]] VirtualTable {
public:
  /// @brief Renders this table value as single-line SQL text. See also @ref
  /// ToStringFormatted for a multi-line, indented rendering.
  virtual std::string ToString() const = 0;

  /// @brief Renders this table value as multi-line, indented SQL text for
  /// readability (e.g. logging/debugging a generated query). Defaults to
  /// @ref ToString; overridden by @ref SelectExpr to lay out each clause on
  /// its own line.
  virtual std::string ToStringFormatted() const;

  /// @brief Renders this table value as SQL text, parenthesized so it can be
  /// safely embedded as a subquery. Overridden by @ref Table to skip
  /// bracketing, since a bare table name never needs it.
  virtual std::string ToStringBracketed() const;

  /// @brief Renders this table value as a FROM item. Distinct from @ref
  /// ToStringBracketed because PostgreSQL's `table_ref` grammar accepts a bare
  /// join but parenthesizes one only when an alias follows.
  /// @throws std::logic_error for table values that PostgreSQL requires an
  /// alias for, i.e. every subquery; call @ref As first.
  virtual std::string ToStringAsFromItem() const;

  /// @brief Aliases this table value as `this AS name`, usable as a FROM
  /// source. `name` must be a valid (optionally dotted) SQL identifier.
  /// @throws std::invalid_argument if `name` is not a valid identifier.
  Table As(std::string_view name) const;

  /// @brief Converts this table value into a subquery expression, e.g. for
  /// use as a scalar subquery. Only available on rvalues.
  operator Expr() const &&;
};

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

/// @brief Transitional representation for SELECT query
class [[nodiscard]] SelectExpr final : public VirtualTable {
public:
  /// @brief Starts a `SELECT ... FROM tbl` query. `tbl` may be a table, a
  /// join, or anything @ref VirtualTable::As has aliased.
  /// @throws std::logic_error if `tbl` is a subquery without an alias.
  SelectExpr(const VirtualTable &tbl);

  /// @brief Sets the SELECT list to a single entry, replacing any previously
  /// set list.
  SelectExpr Select(SelectItem item) &&;

  /// @brief Sets the SELECT list to a comma-separated list of entries,
  /// replacing any previously set list.
  SelectExpr Select(std::initializer_list<SelectItem> items) &&;

  /// @brief Sets the WHERE clause.
  SelectExpr Where(Condition exp) &&;

  /// @brief Sets the ORDER BY clause to a single term.
  SelectExpr OrderBy(OrderByTerm term) &&;

  /// @brief Sets the ORDER BY clause to a comma-separated list of terms.
  SelectExpr OrderBy(std::initializer_list<OrderByTerm> terms) &&;

  /// @brief Sets the GROUP BY clause to a single expression.
  SelectExpr GroupBy(Expr exp) &&;

  /// @brief Sets the GROUP BY clause to a comma-separated list of
  /// expressions.
  SelectExpr GroupBy(std::initializer_list<Expr> exps) &&;

  /// @brief Sets the HAVING clause.
  SelectExpr Having(Condition exp) &&;

  /// @brief Sets the LIMIT clause.
  SelectExpr Limit(int limit) &&;

  /// @brief Sets the OFFSET clause.
  SelectExpr Offset(int offset) &&;

  /// @throws std::logic_error if the SELECT or FROM clause was not set.
  std::string ToString() const override;

  /// @brief Renders the query with each clause on its own line, indented,
  /// e.g.:
  /// ```
  /// SELECT
  ///     a,
  ///     b
  /// FROM
  ///     users
  /// WHERE
  ///     a > b
  /// ```
  /// @throws std::logic_error if the SELECT or FROM clause was not set.
  std::string ToStringFormatted() const override;

private:
  void EnsureValid() const;

  std::string from_;
  std::vector<std::string> select_;
  std::string where_;
  std::vector<std::string> group_by_;
  std::string having_;
  std::vector<std::string> order_by_;
  std::string limit_;
  std::string offset_;
};

/// @brief Handy fabric for @ref SelectExpr
SelectExpr From(const VirtualTable &tbl);

/// @brief Transitional representation for DELETE FROM query
class [[nodiscard]] DeleteFrom final : public VirtualTable {
public:
  /// @brief Starts a `DELETE FROM tbl` query.
  DeleteFrom(const Table &tbl);

  /// @brief Sets the WHERE clause.
  DeleteFrom Where(Condition exp) &&;

  /// @throws std::logic_error if the FROM clause was not set.
  std::string ToString() const override;

private:
  std::string from_;
  std::string where_;
};

/// @brief Transitional representation for INSERT INTO query
class [[nodiscard]] InsertInto final {
public:
  /// @brief Starts an `INSERT INTO tbl` query.
  InsertInto(const Table &tbl);

  /// @brief Sets the list of columns to insert into, replacing any previously
  /// set list.
  /// @throws std::logic_error if the count does not match an already-set
  /// @ref Values list.
  InsertInto Columns(std::initializer_list<Expr> cols) &&;

  /// @brief Sets the list of values to insert, matching @ref Columns by
  /// position and replacing any previously set list.
  /// @throws std::logic_error if the count does not match an already-set
  /// @ref Columns list.
  InsertInto Values(std::initializer_list<Expr> vals) &&;

  /// @throws std::logic_error if no columns or no values were set, or if the
  /// two lists have different lengths.
  std::string ToString() const;

private:
  std::string into_;
  std::vector<std::string> columns_;
  std::vector<std::string> values_;
};

/// @brief Transitional representation for UPDATE query
class [[nodiscard]] Update final {
public:
  /// @brief Starts an `UPDATE tbl` query.
  Update(const Table &tbl);

  /// @brief Adds a `column = value` assignment to the SET clause.
  Update Set(const Expr &column, const Expr &value) &&;

  /// @brief Sets the WHERE clause.
  Update Where(Condition exp) &&;

  /// @throws std::logic_error if no SET assignment was added.
  std::string ToString() const;

private:
  std::string table_;
  std::string set_;
  std::string where_;
};

/// @brief A collation name for use with @ref Expr::Collate.
class [[nodiscard]] Collation final {
public:
  /// @brief Wraps a trusted, developer-written collation name verbatim.
  /// Never pass untrusted/dynamic data here.
  static Collation FromRaw(std::string name);

  /// @brief Renders the collation name as SQL text.
  std::string ToString() const;

private:
  Collation(std::string name);

  std::string name_;
};

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
  SetOp(const VirtualTable &a, const VirtualTable &b, const SetOpKind &kind);

  std::string ToString() const override;

private:
  std::string a_, b_;
  std::string kind_;
};

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

// userver (???) PostgreSQL part:

/// @brief SQL table column
struct [[nodiscard]] Column final {
  /// Column name
  std::string name;
  /// Type as defined in current SQL dialect
  std::string type;
  /// Whether column value can be NULL
  bool is_nullable{false};

  /// @brief Converts this column reference into an Expr wrapping its name.
  operator Expr() const;

  /// @brief Names this column in a SELECT list: `this AS alias`.
  /// @throws std::invalid_argument if `alias` is not a valid identifier.
  SelectItem As(std::string_view alias) const;

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
};

/// @brief Table with associated columns. Usually not created by hands.
class [[nodiscard]] TableWithColumns final : public Table {
public:
  /// @brief Attaches a set of columns to a trusted, developer-written table
  /// name. Never pass untrusted/dynamic data as `name`.
  static TableWithColumns FromRaw(std::string name,
                                  std::vector<Column> columns);

  /// @brief Attaches a set of columns to a trusted, developer-written table
  /// name. Never pass untrusted/dynamic data as `name`.
  static TableWithColumns FromRaw(std::string name,
                                  std::initializer_list<Column> columns);

  /// @brief Builds a comma-separated Expr listing all column names, for use
  /// as a SELECT list.
  Expr SelectArgAll() const;

private:
  TableWithColumns(std::string name, std::vector<Column> columns);
  TableWithColumns(std::string name, std::initializer_list<Column> columns);

  std::vector<Column> columns_;
};

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
