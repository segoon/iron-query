#pragma once

#include <cassert>
#include <string>
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

/// @brief Arbitrary SQL expression
class [[nodiscard]] Expr final {
public:
  Expr(std::string s)
      : expr_(std::move(s)), precedence_(OperatorPrecedence::kSymbol) {}

  Expr(const char *s) : expr_(s), precedence_(OperatorPrecedence::kSymbol) {}

  Expr(int i) : Expr(std::to_string(i)) {}

  Expr(std::string expr, OperatorPrecedence precedence)
      : expr_(std::move(expr)), precedence_(precedence) {}

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

  Expr Like(const Expr &a) const {
    return Expr(Extract(OperatorPrecedence::kBetween) + " LIKE " +
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

  Expr operator<(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) + " < " +
                    other.Extract(OperatorPrecedence::kCompare),
                OperatorPrecedence::kCompare);
  }

  Expr operator>(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) + " > " +
                    other.Extract(OperatorPrecedence::kCompare),
                OperatorPrecedence::kCompare);
  }

  Expr operator==(const Expr &other) const {
    return Expr(Extract(OperatorPrecedence::kCompare) + " = " +
                    other.Extract(OperatorPrecedence::kCompare),
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
    else
      return expr_;
  }

  std::string ToString() const { return Extract(OperatorPrecedence::kExtract); }

private:
  OperatorPrecedence precedence_;
  std::string expr_;
};

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

class [[nodiscard]] TableAlias final {
public:
  TableAlias(std::string_view alias) : alias_(alias) {}

  std::string Dot(const std::string &column) const {
    return alias_ + "." + column;
  }

private:
  std::string alias_;
};

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

  operator Expr() && {
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

  std::string ToString() const override {
    assert(!select_.empty());
    assert(!from_.empty());

    auto s = "SELECT " + select_ + " FROM " + from_;
    if (!where_.empty())
      s += " WHERE " + where_;
    if (!order_by_.empty())
      s += " ORDER BY " + order_by_;
    return s;
  }

private:
  std::string from_;
  std::string select_;
  std::string where_;
  std::string order_by_;
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
    assert(!from_.empty());

    auto s = "DELETE FROM " + from_;
    if (!where_.empty())
      s += " WHERE " + where_;
    return s;
  }

private:
  std::string from_;
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

  Expr operator>(const Expr &) const;

  Expr operator==(const Expr &) const;
};

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

} // namespace sql_builder_pp
