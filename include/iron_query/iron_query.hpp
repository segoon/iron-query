#pragma once

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace iron_query {

// From
// https://www.postgresql.org/docs/current/sql-syntax-lexical.html#SQL-PRECEDENCE
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
  Expr(std::string s);

  Expr(const char *s);

  Expr(int i);

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

  static Expr Count(const Expr &arg);

  /// @brief COUNT(*), since "*" is not a valid Expr argument.
  static Expr CountAll();

  static Expr Sum(const Expr &arg);

  static Expr Avg(const Expr &arg);

  static Expr Min(const Expr &arg);

  static Expr Max(const Expr &arg);

  Expr Dot(const Expr &other) const;

  Expr Cast(const std::string &type) const;

  Expr Collate(const std::string &collation) const;

  Expr operator[](const Expr &other) const;

  Expr operator^(const Expr &other) const;

  Expr Between(const Expr &a, const Expr &b) const;

  Expr NotBetween(const Expr &a, const Expr &b) const;

  Expr Like(const Expr &a) const;

  Expr NotLike(const Expr &a) const;

  Expr In(const Expr &a) const;

  Expr NotIn(const Expr &a) const;

  Expr IsTrue() const;

  Expr IsFalse() const;

  Expr IsNull() const;

  Expr IsNotNull() const;

  Expr operator<(const Expr &other) const;

  Expr operator<=(const Expr &other) const;

  Expr operator>(const Expr &other) const;

  Expr operator>=(const Expr &other) const;

  Expr operator==(const Expr &other) const;

  Expr operator!=(const Expr &other) const;

  Expr operator||(const Expr &other) const;

  Expr operator&&(const Expr &other) const;

  Expr operator+(const Expr &other) const;

  Expr operator-(const Expr &other) const;

  Expr operator*(const Expr &other) const;

  Expr operator/(const Expr &other) const;

  Expr operator%(const Expr &other) const;

  Expr operator!() const;

  std::string Extract(OperatorPrecedence precedence) const;

  std::string ToString() const;

private:
  OperatorPrecedence precedence_;
  std::string expr_;
};

/// @brief Transitional representation for CASE WHEN ... END
class [[nodiscard]] CaseBuilder final {
public:
  CaseBuilder() = default;

  CaseBuilder When(Expr cond) &&;

  CaseBuilder Then(Expr result) &&;

  CaseBuilder Else(Expr result) &&;

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
  // TODO: formatted string, e.g.:
  //
  // FROM
  //     users
  // SELECT
  //     a,
  //     b
  // WHERE
  //     a > b
  virtual std::string ToString() const = 0;

  virtual std::string ToStringBracketed() const;

  Table As(std::string_view name) const;

  operator Expr() const &&;
};

/// @brief Table with name
class [[nodiscard]] Table /* not final! */ : public VirtualTable {
public:
  Table(std::string name);

  std::string ToString() const override;

  std::string ToStringBracketed() const override;

private:
  std::string name_;
};

/// @brief Transitional representation for SELECT query
class [[nodiscard]] SelectExpr final : public VirtualTable {
public:
  SelectExpr(const Table &tbl);

  SelectExpr Select(Expr exp) &&;

  SelectExpr Select(std::initializer_list<Expr> exps) &&;

  SelectExpr Where(Expr exp) &&;

  SelectExpr OrderBy(std::string_view by) &&;

  SelectExpr OrderBy(std::initializer_list<std::string_view> by) &&;

  SelectExpr GroupBy(std::string_view by) &&;

  SelectExpr GroupBy(std::initializer_list<std::string_view> by) &&;

  SelectExpr Having(Expr exp) &&;

  SelectExpr Limit(int limit) &&;

  SelectExpr Offset(int offset) &&;

  std::string ToString() const override;

private:
  std::string from_;
  std::string select_;
  std::string where_;
  std::string group_by_;
  std::string having_;
  std::string order_by_;
  std::string limit_;
  std::string offset_;
};

/// @brief Handy fabric for @ref SelectExpr
SelectExpr From(const Table &tbl);

/// @brief Transitional representation for DELETE FROM query
class [[nodiscard]] DeleteFrom final : public VirtualTable {
public:
  DeleteFrom(const Table &tbl);

  DeleteFrom Where(Expr exp) &&;

  std::string ToString() const override;

private:
  std::string from_;
  std::string where_;
};

/// @brief Transitional representation for INSERT INTO query
class [[nodiscard]] InsertInto final {
public:
  InsertInto(const Table &tbl);

  InsertInto Columns(std::initializer_list<Expr> cols) &&;

  InsertInto Values(std::initializer_list<Expr> vals) &&;

  std::string ToString() const;

private:
  std::string into_;
  std::string columns_;
  std::string values_;
};

/// @brief Transitional representation for UPDATE query
class [[nodiscard]] Update final {
public:
  Update(const Table &tbl);

  Update Set(const Expr &column, const Expr &value) &&;

  Update Where(Expr exp) &&;

  std::string ToString() const;

private:
  std::string table_;
  std::string set_;
  std::string where_;
};

struct [[nodiscard]] JoinKind {
  virtual std::string_view ToString() const = 0;
};

struct [[nodiscard]] Inner final : JoinKind {
  std::string_view ToString() const override;
};
struct [[nodiscard]] Cross final : JoinKind {
  std::string_view ToString() const override;
};
struct [[nodiscard]] LeftOuter final : JoinKind {
  std::string_view ToString() const override;
};
struct [[nodiscard]] RightOuter final : JoinKind {
  std::string_view ToString() const override;
};
struct [[nodiscard]] FullOuter final : JoinKind {
  std::string_view ToString() const override;
};

/// @brief Transitional representation for JOIN query
class [[nodiscard]] Join final : public VirtualTable {
public:
  Join(const VirtualTable &a, const VirtualTable &b, const JoinKind &kind);

  Join On(Expr exp) &&;

  std::string ToString() const override;

private:
  std::string kind_;
  std::string a_, b_;
  std::string on_;
};

struct [[nodiscard]] SetOpKind {
  virtual std::string_view ToString() const = 0;
};

struct [[nodiscard]] Union final : SetOpKind {
  std::string_view ToString() const override;
};
struct [[nodiscard]] UnionAll final : SetOpKind {
  std::string_view ToString() const override;
};
struct [[nodiscard]] Intersect final : SetOpKind {
  std::string_view ToString() const override;
};
struct [[nodiscard]] Except final : SetOpKind {
  std::string_view ToString() const override;
};

/// @brief Transitional representation for UNION/UNION ALL/INTERSECT/EXCEPT
class [[nodiscard]] SetOp final : public VirtualTable {
public:
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
  WithBuilder(std::string name, const VirtualTable &query);

  WithBuilder With(std::string name, const VirtualTable &query) &&;

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

  operator Expr() const;

  Expr operator<(const Expr &other) const;
  Expr operator<=(const Expr &other) const;
  Expr operator>(const Expr &other) const;
  Expr operator>=(const Expr &other) const;
  Expr operator==(const Expr &other) const;
  Expr operator!=(const Expr &other) const;
};

/// @brief Table with associated columns. Usually not created by hands.
class [[nodiscard]] TableWithColumns final : public Table {
public:
  TableWithColumns(std::string name, std::vector<Column> columns);

  TableWithColumns(std::string name, std::initializer_list<Column> columns);

  Expr SelectArgAll() const;

private:
  std::vector<Column> columns_;
};

class [[nodiscard]] TableAlias final {
public:
  TableAlias(std::string_view alias);

  std::string Dot(const std::string &column) const;

  std::string Dot(const Column &column) const;

private:
  std::string alias_;
};

} // namespace iron_query
