#pragma once

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
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

/// @brief Arbitrary SQL expression
class [[nodiscard]] Expr final {
public:
  /// @brief Wraps a trusted, developer-written SQL fragment verbatim.
  Expr(std::string s);

  /// @brief Wraps a trusted, developer-written SQL fragment verbatim.
  Expr(const char *s);

  /// @brief Wraps an integer literal.
  Expr(int i);

  /// @brief Wraps a trusted SQL fragment together with the precedence of its
  /// top-level operator, so that @ref Extract can bracket it correctly when
  /// it is embedded into a higher-precedence expression.
  Expr(std::string expr, OperatorPrecedence precedence);

  /// @brief Builds a properly escaped and quoted SQL string literal out of an
  /// arbitrary (possibly untrusted) value. Use this instead of Expr(string)
  /// whenever the content is not a trusted, developer-written SQL fragment.
  static Expr Literal(const std::string &value);

  /// @brief Builds a properly escaped and quoted SQL identifier out of an
  /// arbitrary (possibly untrusted/dynamic) name. Use this instead of
  /// Expr(string) whenever a table/column name is not a trusted,
  /// developer-written literal.
  static Expr Ident(const std::string &name);

  /// @brief Builds a function call expression, e.g. Expr::Call("COALESCE",
  /// {a, b}) -> "COALESCE(a, b)".
  static Expr Call(const std::string &name, std::initializer_list<Expr> args);

  /// @brief EXISTS (subquery).
  static Expr Exists(const VirtualTable &subquery);

  /// @brief NOT EXISTS (subquery).
  static Expr NotExists(const VirtualTable &subquery);

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

  /// @brief SQL type cast: `CAST (this AS type)`.
  Expr Cast(const std::string &type) const;

  /// @brief `this COLLATE collation`.
  Expr Collate(const std::string &collation) const;

  /// @brief Array/index access: `this[other]`.
  Expr operator[](const Expr &other) const;

  /// @brief Exponentiation: `this ^ other`.
  Expr operator^(const Expr &other) const;

  /// @brief `this BETWEEN a AND b`.
  Expr Between(const Expr &a, const Expr &b) const;

  /// @brief `this NOT BETWEEN a AND b`.
  Expr NotBetween(const Expr &a, const Expr &b) const;

  /// @brief `this LIKE a`.
  Expr Like(const Expr &a) const;

  /// @brief `this NOT LIKE a`.
  Expr NotLike(const Expr &a) const;

  /// @brief `this IN a`.
  Expr In(const Expr &a) const;

  /// @brief `this NOT IN a`.
  Expr NotIn(const Expr &a) const;

  /// @brief `this IS TRUE`.
  Expr IsTrue() const;

  /// @brief `this IS FALSE`.
  Expr IsFalse() const;

  /// @brief `this IS NULL`.
  Expr IsNull() const;

  /// @brief `this IS NOT NULL`.
  Expr IsNotNull() const;

  /// @brief `this < other`.
  Expr operator<(const Expr &other) const;

  /// @brief `this <= other`.
  Expr operator<=(const Expr &other) const;

  /// @brief `this > other`.
  Expr operator>(const Expr &other) const;

  /// @brief `this >= other`.
  Expr operator>=(const Expr &other) const;

  /// @brief `this = other`.
  Expr operator==(const Expr &other) const;

  /// @brief `this != other`.
  Expr operator!=(const Expr &other) const;

  /// @brief `this OR other`.
  Expr operator||(const Expr &other) const;

  /// @brief `this AND other`.
  Expr operator&&(const Expr &other) const;

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

  /// @brief `NOT this`.
  Expr operator!() const;

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
  OperatorPrecedence precedence_;
  std::string expr_;
};

/// @brief Transitional representation for CASE WHEN ... END
class [[nodiscard]] CaseBuilder final {
public:
  /// @brief Starts an empty CASE expression; call @ref When to add branches.
  CaseBuilder() = default;

  /// @brief Begins a new `WHEN cond` branch.
  /// @throws std::logic_error if called twice in a row without a matching
  /// @ref Then in between.
  CaseBuilder When(Expr cond) &&;

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

  /// @brief Aliases this table value as `(this) AS name`, usable as a FROM
  /// source.
  Table As(std::string_view name) const;

  /// @brief Converts this table value into a subquery expression, e.g. for
  /// use as a scalar subquery. Only available on rvalues.
  operator Expr() const &&;
};

/// @brief Table with name
class [[nodiscard]] Table /* not final! */ : public VirtualTable {
public:
  /// @brief Wraps a trusted, developer-written table name/reference.
  Table(std::string name);

  std::string ToString() const override;

  /// @brief Returns the name as-is: a bare table name never needs brackets.
  std::string ToStringBracketed() const override;

private:
  std::string name_;
};

/// @brief Transitional representation for SELECT query
class [[nodiscard]] SelectExpr final : public VirtualTable {
public:
  /// @brief Starts a `SELECT ... FROM tbl` query.
  SelectExpr(const Table &tbl);

  /// @brief Sets the SELECT list to a single expression.
  SelectExpr Select(Expr exp) &&;

  /// @brief Sets the SELECT list to a comma-separated list of expressions.
  SelectExpr Select(std::initializer_list<Expr> exps) &&;

  /// @brief Sets the WHERE clause.
  SelectExpr Where(Expr exp) &&;

  /// @brief Sets the ORDER BY clause to a single (trusted) expression.
  SelectExpr OrderBy(std::string_view by) &&;

  /// @brief Sets the ORDER BY clause to a comma-separated list of (trusted)
  /// expressions.
  SelectExpr OrderBy(std::initializer_list<std::string_view> by) &&;

  /// @brief Sets the GROUP BY clause to a single (trusted) expression.
  SelectExpr GroupBy(std::string_view by) &&;

  /// @brief Sets the GROUP BY clause to a comma-separated list of (trusted)
  /// expressions.
  SelectExpr GroupBy(std::initializer_list<std::string_view> by) &&;

  /// @brief Sets the HAVING clause.
  SelectExpr Having(Expr exp) &&;

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
SelectExpr From(const Table &tbl);

/// @brief Transitional representation for DELETE FROM query
class [[nodiscard]] DeleteFrom final : public VirtualTable {
public:
  /// @brief Starts a `DELETE FROM tbl` query.
  DeleteFrom(const Table &tbl);

  /// @brief Sets the WHERE clause.
  DeleteFrom Where(Expr exp) &&;

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

  /// @brief Sets the list of columns to insert into.
  InsertInto Columns(std::initializer_list<Expr> cols) &&;

  /// @brief Sets the list of values to insert, matching @ref Columns by
  /// position.
  InsertInto Values(std::initializer_list<Expr> vals) &&;

  /// @throws std::logic_error if no columns or no values were set.
  std::string ToString() const;

private:
  std::string into_;
  std::string columns_;
  std::string values_;
};

/// @brief Transitional representation for UPDATE query
class [[nodiscard]] Update final {
public:
  /// @brief Starts an `UPDATE tbl` query.
  Update(const Table &tbl);

  /// @brief Adds a `column = value` assignment to the SET clause.
  Update Set(const Expr &column, const Expr &value) &&;

  /// @brief Sets the WHERE clause.
  Update Where(Expr exp) &&;

  /// @throws std::logic_error if no SET assignment was added.
  std::string ToString() const;

private:
  std::string table_;
  std::string set_;
  std::string where_;
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
  Join On(Expr exp) &&;

  std::string ToString() const override;

private:
  std::string kind_;
  std::string a_, b_;
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
  std::string kind_;
  std::string a_, b_;
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
  /// @brief Starts a WITH clause with a single CTE: `name AS (query)`.
  WithBuilder(std::string name, const VirtualTable &query);

  /// @brief Adds another CTE: `, name AS (query)`.
  WithBuilder With(std::string name, const VirtualTable &query) &&;

  /// @brief Finalizes the WITH clause with the main query that follows it.
  WithQuery Main(const VirtualTable &query) &&;

private:
  std::string ctes_;
};

/// @brief Handy fabric for @ref WithBuilder
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

  /// @brief `this < other`.
  Expr operator<(const Expr &other) const;
  /// @brief `this <= other`.
  Expr operator<=(const Expr &other) const;
  /// @brief `this > other`.
  Expr operator>(const Expr &other) const;
  /// @brief `this >= other`.
  Expr operator>=(const Expr &other) const;
  /// @brief `this = other`.
  Expr operator==(const Expr &other) const;
  /// @brief `this != other`.
  Expr operator!=(const Expr &other) const;
};

/// @brief Table with associated columns. Usually not created by hands.
class [[nodiscard]] TableWithColumns final : public Table {
public:
  /// @brief Attaches a set of columns to a table name.
  TableWithColumns(std::string name, std::vector<Column> columns);

  /// @brief Attaches a set of columns to a table name.
  TableWithColumns(std::string name, std::initializer_list<Column> columns);

  /// @brief Builds a comma-separated Expr listing all column names, for use
  /// as a SELECT list.
  Expr SelectArgAll() const;

private:
  std::vector<Column> columns_;
};

/// @brief A table alias, usable to qualify column references as
/// `alias.column`.
class [[nodiscard]] TableAlias final {
public:
  /// @brief Wraps a trusted, developer-written alias name.
  TableAlias(std::string_view alias);

  /// @brief Qualifies a column name as `alias.column`.
  std::string Dot(const std::string &column) const;

  /// @brief Qualifies a column as `alias.column.name`.
  std::string Dot(const Column &column) const;

private:
  std::string alias_;
};

} // namespace iron_query
