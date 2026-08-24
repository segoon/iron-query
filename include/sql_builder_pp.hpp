#pragma once

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace sql_builder_pp {

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
  Expr(std::string s)
      : expr_(std::move(s)), precedence_(OperatorPrecedence::kSymbol) {}

  Expr(const char *s) : expr_(s), precedence_(OperatorPrecedence::kSymbol) {}

  Expr(int i) : Expr(std::to_string(i)) {}

  Expr(std::string expr, OperatorPrecedence precedence)
      : expr_(std::move(expr)), precedence_(precedence) {}

  /// @brief Builds a properly escaped and quoted SQL string literal out of an
  /// arbitrary (possibly untrusted) value. Use this instead of Expr(string)
  /// whenever the content is not a trusted, developer-written SQL fragment.
  static Expr Literal(const std::string &value) {
    if (value.find('\0') != std::string::npos)
      throw std::invalid_argument(
          "sql_builder_pp: string literal must not contain NUL bytes");

    std::string escaped = "'";
    for (char c : value) {
      if (c == '\'')
        escaped += '\'';
      escaped += c;
    }
    escaped += "'";
    return Expr(std::move(escaped), OperatorPrecedence::kSymbol);
  }

  /// @brief Builds a properly escaped and quoted SQL identifier out of an
  /// arbitrary (possibly untrusted/dynamic) name. Use this instead of
  /// Expr(string) whenever a table/column name is not a trusted,
  /// developer-written literal.
  static Expr Ident(const std::string &name) {
    if (name.find('\0') != std::string::npos)
      throw std::invalid_argument(
          "sql_builder_pp: identifier must not contain NUL bytes");

    std::string escaped = "\"";
    for (char c : name) {
      if (c == '"')
        escaped += '"';
      escaped += c;
    }
    escaped += "\"";
    return Expr(std::move(escaped), OperatorPrecedence::kSymbol);
  }

  /// @brief Builds a function call expression, e.g. Expr::Call("COALESCE",
  /// {a, b}) -> "COALESCE(a, b)".
  static Expr Call(const std::string &name, std::initializer_list<Expr> args) {
    std::string s = name + "(";
    bool first = true;
    for (const auto &arg : args) {
      if (!first)
        s += ", ";
      s += arg.ToString();
      first = false;
    }
    s += ")";
    return Expr(std::move(s), OperatorPrecedence::kSymbol);
  }

  /// @brief EXISTS (subquery). Defined below, after VirtualTable.
  static Expr Exists(const VirtualTable &subquery);

  /// @brief NOT EXISTS (subquery). Defined below, after VirtualTable.
  static Expr NotExists(const VirtualTable &subquery);

  static Expr Count(const Expr &arg) { return Call("COUNT", {arg}); }

  /// @brief COUNT(*), since "*" is not a valid Expr argument.
  static Expr CountAll() { return Expr("COUNT(*)"); }

  static Expr Sum(const Expr &arg) { return Call("SUM", {arg}); }

  static Expr Avg(const Expr &arg) { return Call("AVG", {arg}); }

  static Expr Min(const Expr &arg) { return Call("MIN", {arg}); }

  static Expr Max(const Expr &arg) { return Call("MAX", {arg}); }

  Expr Dot(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kDot) + "." +
                    other.Extract(OperatorPrecedence::kDot),
                OperatorPrecedence::kDot);
  }

  Expr Cast(const std::string &type) const {
    return Expr("CAST (" + Extract(OperatorPrecedence::kTypecast) + " AS " +
                    type + ")",
                OperatorPrecedence::kTypecast);
  }

  Expr Collate(const std::string &collation) const {
    return Expr(Extract(OperatorPrecedence::kCollate) + " COLLATE " + collation,
                OperatorPrecedence::kCollate);
  }

  Expr operator[](const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kIndex) + "[" + other.ToString() +
                "]");
  }

  Expr operator^(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kExp) + " ^ " +
                    other.Extract(OperatorPrecedence::kExp),
                OperatorPrecedence::kExp);
  }

  Expr Between(const Expr &a, const Expr &b) const {
    return Expr(Extract(OperatorPrecedence::kBetween) + " BETWEEN " +
                    a.Extract(OperatorPrecedence::kBetween) + " AND " +
                    b.Extract(OperatorPrecedence::kBetween),
                OperatorPrecedence::kBetween);
  }

  Expr NotBetween(const Expr &a, const Expr &b) const {
    return Expr(Extract(OperatorPrecedence::kBetween) + " NOT BETWEEN " +
                    a.Extract(OperatorPrecedence::kBetween) + " AND " +
                    b.Extract(OperatorPrecedence::kBetween),
                OperatorPrecedence::kBetween);
  }

  Expr Like(const Expr &a) const {
    return Expr(Extract(OperatorPrecedence::kBetween) + " LIKE " +
                    a.Extract(OperatorPrecedence::kBetween),
                OperatorPrecedence::kBetween);
  }

  Expr NotLike(const Expr &a) const {
    return Expr(Extract(OperatorPrecedence::kBetween) + " NOT LIKE " +
                    a.Extract(OperatorPrecedence::kBetween),
                OperatorPrecedence::kBetween);
  }

  Expr In(const Expr &a) const {
    return Expr(Extract(OperatorPrecedence::kBetween) + " IN " +
                    a.Extract(OperatorPrecedence::kBetween),
                OperatorPrecedence::kBetween);
  }

  Expr NotIn(const Expr &a) const {
    return Expr(Extract(OperatorPrecedence::kBetween) + " NOT IN " +
                    a.Extract(OperatorPrecedence::kBetween),
                OperatorPrecedence::kBetween);
  }

  Expr IsTrue() const {
    return Expr(Extract(OperatorPrecedence::kIs) + " IS TRUE",
                OperatorPrecedence::kIs);
  }

  Expr IsFalse() const {
    return Expr(Extract(OperatorPrecedence::kIs) + " IS FALSE",
                OperatorPrecedence::kIs);
  }

  Expr IsNull() const {
    return Expr(Extract(OperatorPrecedence::kIs) + " IS NULL",
                OperatorPrecedence::kIs);
  }

  Expr IsNotNull() const {
    return Expr(Extract(OperatorPrecedence::kIs) + " IS NOT NULL",
                OperatorPrecedence::kIs);
  }

  Expr operator<(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) + " < " +
                    other.Extract(OperatorPrecedence::kCompare),
                OperatorPrecedence::kCompare);
  }

  Expr operator<=(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) +
                    " <= " + other.Extract(OperatorPrecedence::kCompare),
                OperatorPrecedence::kCompare);
  }

  Expr operator>(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) + " > " +
                    other.Extract(OperatorPrecedence::kCompare),
                OperatorPrecedence::kCompare);
  }

  Expr operator>=(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) +
                    " >= " + other.Extract(OperatorPrecedence::kCompare),
                OperatorPrecedence::kCompare);
  }

  Expr operator==(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) + " = " +
                    other.Extract(OperatorPrecedence::kCompare),
                OperatorPrecedence::kCompare);
  }

  Expr operator!=(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) +
                    " != " + other.Extract(OperatorPrecedence::kCompare),
                OperatorPrecedence::kCompare);
  }

  Expr operator||(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kOr) + " OR " +
                    other.Extract(OperatorPrecedence::kOr),
                OperatorPrecedence::kOr);
  }

  Expr operator&&(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kAnd) + " AND " +
                    other.Extract(OperatorPrecedence::kAnd),
                OperatorPrecedence::kAnd);
  }

  Expr operator+(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kPlus) + " + " +
                    other.Extract(OperatorPrecedence::kPlus),
                OperatorPrecedence::kPlus);
  }

  Expr operator-(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kPlus) + " - " +
                    other.Extract(OperatorPrecedence::kPlus),
                OperatorPrecedence::kPlus);
  }

  Expr operator*(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kMul) + " * " +
                    other.Extract(OperatorPrecedence::kMul),
                OperatorPrecedence::kMul);
  }

  Expr operator/(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kMul) + " / " +
                    other.Extract(OperatorPrecedence::kMul),
                OperatorPrecedence::kMul);
  }

  Expr operator%(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kMul) + " % " +
                    other.Extract(OperatorPrecedence::kMul),
                OperatorPrecedence::kMul);
  }

  Expr operator!() const {
    return Expr("NOT " + Extract(OperatorPrecedence::kNot),
                OperatorPrecedence::kNot);
  }

  std::string Extract(OperatorPrecedence precedence) const {
    if (precedence_ >= precedence)
      return "(" + expr_ + ")";
    return expr_;
  }

  std::string ToString() const { return Extract(OperatorPrecedence::kExtract); }

private:
  OperatorPrecedence precedence_;
  std::string expr_;
};

/// @brief Transitional representation for CASE WHEN ... END
class [[nodiscard]] CaseBuilder final {
public:
  CaseBuilder() = default;

  CaseBuilder When(Expr cond) && {
    if (has_pending_when_)
      throw std::logic_error(
          "sql_builder_pp: When() called twice without a matching Then()");
    pending_when_ = cond.ToString();
    has_pending_when_ = true;
    return std::move(*this);
  }

  CaseBuilder Then(Expr result) && {
    if (!has_pending_when_)
      throw std::logic_error("sql_builder_pp: Then() called without a "
                             "preceding When()");
    whens_ += "WHEN " + pending_when_ + " THEN " + result.ToString() + " ";
    has_pending_when_ = false;
    return std::move(*this);
  }

  CaseBuilder Else(Expr result) && {
    else_ = result.ToString();
    return std::move(*this);
  }

  Expr End() const {
    if (whens_.empty())
      throw std::logic_error(
          "sql_builder_pp: CASE requires at least one When()/Then() pair");

    std::string s = "CASE " + whens_;
    if (!else_.empty())
      s += "ELSE " + else_ + " ";
    s += "END";
    return Expr(std::move(s), OperatorPrecedence::kSymbol);
  }

private:
  std::string whens_;
  std::string pending_when_;
  std::string else_;
  bool has_pending_when_{false};
};

/// @brief Handy fabric for @ref CaseBuilder
CaseBuilder Case() { return CaseBuilder(); }

/// @brief A synonym for $1
const Expr _1 = "$1";
const Expr _2 = "$2";
const Expr _3 = "$3";
const Expr _4 = "$4";
const Expr _5 = "$5";
const Expr _6 = "$6";
const Expr _7 = "$7";
const Expr _8 = "$8";
const Expr _9 = "$9";
const Expr _10 = "$10";

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

  virtual std::string ToStringBracketed() const {
    return "(" + ToString() + ")";
  }

  Table As(std::string_view name) const;

  operator Expr() const && {
    return Expr(ToStringBracketed(), OperatorPrecedence::kSymbol);
  }
};

/// @brief Table with name
class [[nodiscard]] Table /* not final! */ : public VirtualTable {
public:
  Table(std::string name) : name_(std::move(name)) {}

  std::string ToString() const override { return name_; }

  std::string ToStringBracketed() const override {
    // It is a table with name, no need to bracket it
    return ToString();
  }

private:
  std::string name_;
};

inline Table VirtualTable::As(std::string_view name) const {
  return Table("(" + ToStringBracketed() + " AS " + std::string(name) + ")");
}

inline Expr Expr::Exists(const VirtualTable &subquery) {
  return Expr("EXISTS " + subquery.ToStringBracketed(),
              OperatorPrecedence::kSymbol);
}

inline Expr Expr::NotExists(const VirtualTable &subquery) {
  return Expr("NOT EXISTS " + subquery.ToStringBracketed(),
              OperatorPrecedence::kSymbol);
}

/// @brief Transitional representation for SELECT query
class [[nodiscard]] SelectExpr final : public VirtualTable {
public:
  SelectExpr(const Table &tbl) : from_(tbl.ToStringBracketed()) {}

  SelectExpr Select(Expr exp) && {
    select_ = exp.ToString();
    return std::move(*this);
  }

  SelectExpr Select(std::initializer_list<Expr> exps) && {
    for (const auto &exp : exps) {
      if (!select_.empty())
        select_ += ", ";
      select_ += exp.ToString();
    }
    return std::move(*this);
  }

  SelectExpr Where(Expr exp) && {
    where_ = exp.ToString();
    return std::move(*this);
  }

  SelectExpr OrderBy(std::string_view by) && {
    order_by_ = by;
    return std::move(*this);
  }

  SelectExpr OrderBy(std::initializer_list<std::string_view> by) && {
    for (const auto &arg : by) {
      if (!order_by_.empty())
        order_by_ += ", ";
      order_by_ += arg;
    }
    return std::move(*this);
  }

  SelectExpr GroupBy(std::string_view by) && {
    group_by_ = by;
    return std::move(*this);
  }

  SelectExpr GroupBy(std::initializer_list<std::string_view> by) && {
    for (const auto &arg : by) {
      if (!group_by_.empty())
        group_by_ += ", ";
      group_by_ += arg;
    }
    return std::move(*this);
  }

  SelectExpr Having(Expr exp) && {
    having_ = exp.ToString();
    return std::move(*this);
  }

  SelectExpr Limit(int limit) && {
    limit_ = std::to_string(limit);
    return std::move(*this);
  }

  SelectExpr Offset(int offset) && {
    offset_ = std::to_string(offset);
    return std::move(*this);
  }

  std::string ToString() const override {
    if (select_.empty())
      throw std::logic_error("sql_builder_pp: SELECT clause is not set");
    if (from_.empty())
      throw std::logic_error("sql_builder_pp: FROM clause is not set");

    auto s = "SELECT " + select_ + " FROM " + from_;
    if (!where_.empty())
      s += " WHERE " + where_;
    if (!group_by_.empty())
      s += " GROUP BY " + group_by_;
    if (!having_.empty())
      s += " HAVING " + having_;
    if (!order_by_.empty())
      s += " ORDER BY " + order_by_;
    if (!limit_.empty())
      s += " LIMIT " + limit_;
    if (!offset_.empty())
      s += " OFFSET " + offset_;
    return s;
  }

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
SelectExpr From(const Table &tbl) { return SelectExpr(tbl); }

/// @brief Transitional representation for DELETE FROM query
class [[nodiscard]] DeleteFrom final : public VirtualTable {
public:
  DeleteFrom(const Table &tbl) : from_(tbl.ToStringBracketed()) {}

  DeleteFrom Where(Expr exp) && {
    where_ = exp.ToString();
    return std::move(*this);
  }

  std::string ToString() const override {
    if (from_.empty())
      throw std::logic_error("sql_builder_pp: FROM clause is not set");

    auto s = "DELETE FROM " + from_;
    if (!where_.empty())
      s += " WHERE " + where_;
    return s;
  }

private:
  std::string from_;
  std::string where_;
};

/// @brief Transitional representation for INSERT INTO query
class [[nodiscard]] InsertInto final {
public:
  InsertInto(const Table &tbl) : into_(tbl.ToStringBracketed()) {}

  InsertInto Columns(std::initializer_list<Expr> cols) && {
    for (const auto &col : cols) {
      if (!columns_.empty())
        columns_ += ", ";
      columns_ += col.ToString();
    }
    return std::move(*this);
  }

  InsertInto Values(std::initializer_list<Expr> vals) && {
    for (const auto &val : vals) {
      if (!values_.empty())
        values_ += ", ";
      values_ += val.ToString();
    }
    return std::move(*this);
  }

  std::string ToString() const {
    if (columns_.empty())
      throw std::logic_error("sql_builder_pp: no columns to insert into");
    if (values_.empty())
      throw std::logic_error("sql_builder_pp: no values to insert");

    return "INSERT INTO " + into_ + " (" + columns_ + ") VALUES (" + values_ +
           ")";
  }

private:
  std::string into_;
  std::string columns_;
  std::string values_;
};

/// @brief Transitional representation for UPDATE query
class [[nodiscard]] Update final {
public:
  Update(const Table &tbl) : table_(tbl.ToStringBracketed()) {}

  Update Set(const Expr &column, const Expr &value) && {
    if (!set_.empty())
      set_ += ", ";
    set_ += column.ToString() + " = " + value.ToString();
    return std::move(*this);
  }

  Update Where(Expr exp) && {
    where_ = exp.ToString();
    return std::move(*this);
  }

  std::string ToString() const {
    if (set_.empty())
      throw std::logic_error("sql_builder_pp: SET clause is not set");

    auto s = "UPDATE " + table_ + " SET " + set_;
    if (!where_.empty())
      s += " WHERE " + where_;
    return s;
  }

private:
  std::string table_;
  std::string set_;
  std::string where_;
};

struct [[nodiscard]] JoinKind {
  virtual std::string_view ToString() const = 0;
};

struct [[nodiscard]] Inner final : JoinKind {
  std::string_view ToString() const override { return "INNER"; }
};
struct [[nodiscard]] Cross final : JoinKind {
  std::string_view ToString() const override { return "CROSS"; }
};
struct [[nodiscard]] LeftOuter final : JoinKind {
  std::string_view ToString() const override { return "LEFT OUTER"; }
};
struct [[nodiscard]] RightOuter final : JoinKind {
  std::string_view ToString() const override { return "RIGHT OUTER"; }
};
struct [[nodiscard]] FullOuter final : JoinKind {
  std::string_view ToString() const override { return "FULL OUTER"; }
};

/// @brief Transitional representation for JOIN query
class [[nodiscard]] Join final : public VirtualTable {
public:
  Join(const VirtualTable &a, const VirtualTable &b, const JoinKind &kind)
      : a_(a.ToStringBracketed()), b_(b.ToStringBracketed()),
        kind_(kind.ToString()) {}

  Join On(Expr exp) && {
    on_ = exp.ToString();
    return std::move(*this);
  }

  std::string ToString() const override {
    auto s = a_ + " " + kind_ + " JOIN " + b_;
    if (!on_.empty())
      s += " ON " + on_;
    return s;
  }

private:
  std::string kind_;
  std::string a_, b_;
  std::string on_;
};

struct [[nodiscard]] SetOpKind {
  virtual std::string_view ToString() const = 0;
};

struct [[nodiscard]] Union final : SetOpKind {
  std::string_view ToString() const override { return "UNION"; }
};
struct [[nodiscard]] UnionAll final : SetOpKind {
  std::string_view ToString() const override { return "UNION ALL"; }
};
struct [[nodiscard]] Intersect final : SetOpKind {
  std::string_view ToString() const override { return "INTERSECT"; }
};
struct [[nodiscard]] Except final : SetOpKind {
  std::string_view ToString() const override { return "EXCEPT"; }
};

/// @brief Transitional representation for UNION/UNION ALL/INTERSECT/EXCEPT
class [[nodiscard]] SetOp final : public VirtualTable {
public:
  SetOp(const VirtualTable &a, const VirtualTable &b, const SetOpKind &kind)
      : a_(a.ToStringBracketed()), b_(b.ToStringBracketed()),
        kind_(kind.ToString()) {}

  std::string ToString() const override { return a_ + " " + kind_ + " " + b_; }

private:
  std::string kind_;
  std::string a_, b_;
};

/// @brief Finalized WITH ... query, usable anywhere a VirtualTable is
/// (subquery, FROM source, etc.)
class [[nodiscard]] WithQuery final : public VirtualTable {
public:
  WithQuery(std::string ctes, std::string main)
      : ctes_(std::move(ctes)), main_(std::move(main)) {}

  std::string ToString() const override {
    return "WITH " + ctes_ + " " + main_;
  }

private:
  std::string ctes_;
  std::string main_;
};

/// @brief Transitional representation for a WITH clause being built up
class [[nodiscard]] WithBuilder final {
public:
  WithBuilder(std::string name, const VirtualTable &query)
      : ctes_(std::move(name) + " AS " + query.ToStringBracketed()) {}

  WithBuilder With(std::string name, const VirtualTable &query) && {
    ctes_ += ", " + std::move(name) + " AS " + query.ToStringBracketed();
    return std::move(*this);
  }

  WithQuery Main(const VirtualTable &query) && {
    return WithQuery(std::move(ctes_), query.ToString());
  }

private:
  std::string ctes_;
};

/// @brief Handy fabric for @ref WithBuilder
WithBuilder With(std::string name, const VirtualTable &query) {
  return WithBuilder(std::move(name), query);
}

// userver (???) PostgreSQL part:

/// @brief SQL table column
struct [[nodiscard]] Column final {
  /// Column name
  std::string name;
  /// Type as defined in current SQL dialect
  std::string type;
  /// Whether column value can be NULL
  bool is_nullable{false};

  operator Expr() const { return Expr(name); }

  Expr operator<(const Expr &other) const;
  Expr operator<=(const Expr &other) const;
  Expr operator>(const Expr &other) const;
  Expr operator>=(const Expr &other) const;
  Expr operator==(const Expr &other) const;
  Expr operator!=(const Expr &other) const;
};

inline Expr Column::operator<(const Expr &other) const {
  return Expr(*this) < other;
}

inline Expr Column::operator<=(const Expr &other) const {
  return Expr(*this) <= other;
}

inline Expr Column::operator>(const Expr &other) const {
  return Expr(*this) > other;
}

inline Expr Column::operator>=(const Expr &other) const {
  return Expr(*this) >= other;
}

inline Expr Column::operator==(const Expr &other) const {
  return Expr(*this) == other;
}

inline Expr Column::operator!=(const Expr &other) const {
  return Expr(*this) != other;
}

/// @brief Table with associated columns. Usually not created by hands.
class [[nodiscard]] TableWithColumns final : public Table {
public:
  TableWithColumns(std::string name, std::vector<Column> columns)
      : Table(std::move(name)), columns_(columns) {}

  TableWithColumns(std::string name, std::initializer_list<Column> columns)
      : TableWithColumns(std::move(name), std::vector(columns)) {}

  Expr SelectArgAll() const {
    std::string s;
    for (const auto &col : columns_) {
      if (!s.empty())
        s += ", ";
      s += col.name;
    }
    return Expr(s);
  }

private:
  std::vector<Column> columns_;
};

class [[nodiscard]] TableAlias final {
public:
  TableAlias(std::string_view alias) : alias_(alias) {}

  std::string Dot(const std::string &column) const {
    return alias_ + "." + column;
  }

  std::string Dot(const Column &column) const { return Dot(column.name); }

private:
  std::string alias_;
};

} // namespace sql_builder_pp
