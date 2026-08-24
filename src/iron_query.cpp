#include <iron_query/iron_query.hpp>

namespace iron_query {

// ---------------------------------------------------------------------------
// Expr
// ---------------------------------------------------------------------------

Expr::Expr(std::string s)
    : expr_(std::move(s)), precedence_(OperatorPrecedence::kSymbol) {}

Expr::Expr(const char *s)
    : expr_(s), precedence_(OperatorPrecedence::kSymbol) {}

Expr::Expr(int i) : Expr(std::to_string(i)) {}

Expr::Expr(std::string expr, OperatorPrecedence precedence)
    : expr_(std::move(expr)), precedence_(precedence) {}

Expr Expr::Literal(const std::string &value) {
  if (value.find('\0') != std::string::npos)
    throw std::invalid_argument(
        "iron_query: string literal must not contain NUL bytes");

  std::string escaped = "'";
  for (char c : value) {
    if (c == '\'')
      escaped += '\'';
    escaped += c;
  }
  escaped += "'";
  return Expr(std::move(escaped), OperatorPrecedence::kSymbol);
}

Expr Expr::Ident(const std::string &name) {
  if (name.find('\0') != std::string::npos)
    throw std::invalid_argument(
        "iron_query: identifier must not contain NUL bytes");

  std::string escaped = "\"";
  for (char c : name) {
    if (c == '"')
      escaped += '"';
    escaped += c;
  }
  escaped += "\"";
  return Expr(std::move(escaped), OperatorPrecedence::kSymbol);
}

Expr Expr::Call(const std::string &name, std::initializer_list<Expr> args) {
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

Expr Expr::Exists(const VirtualTable &subquery) {
  return Expr("EXISTS " + subquery.ToStringBracketed(),
              OperatorPrecedence::kSymbol);
}

Expr Expr::NotExists(const VirtualTable &subquery) {
  return Expr("NOT EXISTS " + subquery.ToStringBracketed(),
              OperatorPrecedence::kSymbol);
}

Expr Expr::Count(const Expr &arg) { return Call("COUNT", {arg}); }

Expr Expr::CountAll() { return Expr("COUNT(*)"); }

Expr Expr::Sum(const Expr &arg) { return Call("SUM", {arg}); }

Expr Expr::Avg(const Expr &arg) { return Call("AVG", {arg}); }

Expr Expr::Min(const Expr &arg) { return Call("MIN", {arg}); }

Expr Expr::Max(const Expr &arg) { return Call("MAX", {arg}); }

Expr Expr::Dot(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kDot) + "." +
                  other.Extract(OperatorPrecedence::kDot),
              OperatorPrecedence::kDot);
}

Expr Expr::Cast(const std::string &type) const {
  return Expr("CAST (" + Extract(OperatorPrecedence::kTypecast) + " AS " +
                  type + ")",
              OperatorPrecedence::kTypecast);
}

Expr Expr::Collate(const std::string &collation) const {
  return Expr(Extract(OperatorPrecedence::kCollate) + " COLLATE " + collation,
              OperatorPrecedence::kCollate);
}

Expr Expr::operator[](const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kIndex) + "[" + other.ToString() +
              "]");
}

Expr Expr::operator^(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kExp) + " ^ " +
                  other.Extract(OperatorPrecedence::kExp),
              OperatorPrecedence::kExp);
}

Expr Expr::Between(const Expr &a, const Expr &b) const {
  return Expr(Extract(OperatorPrecedence::kBetween) + " BETWEEN " +
                  a.Extract(OperatorPrecedence::kBetween) + " AND " +
                  b.Extract(OperatorPrecedence::kBetween),
              OperatorPrecedence::kBetween);
}

Expr Expr::NotBetween(const Expr &a, const Expr &b) const {
  return Expr(Extract(OperatorPrecedence::kBetween) + " NOT BETWEEN " +
                  a.Extract(OperatorPrecedence::kBetween) + " AND " +
                  b.Extract(OperatorPrecedence::kBetween),
              OperatorPrecedence::kBetween);
}

Expr Expr::Like(const Expr &a) const {
  return Expr(Extract(OperatorPrecedence::kBetween) + " LIKE " +
                  a.Extract(OperatorPrecedence::kBetween),
              OperatorPrecedence::kBetween);
}

Expr Expr::NotLike(const Expr &a) const {
  return Expr(Extract(OperatorPrecedence::kBetween) + " NOT LIKE " +
                  a.Extract(OperatorPrecedence::kBetween),
              OperatorPrecedence::kBetween);
}

Expr Expr::In(const Expr &a) const {
  return Expr(Extract(OperatorPrecedence::kBetween) + " IN " +
                  a.Extract(OperatorPrecedence::kBetween),
              OperatorPrecedence::kBetween);
}

Expr Expr::NotIn(const Expr &a) const {
  return Expr(Extract(OperatorPrecedence::kBetween) + " NOT IN " +
                  a.Extract(OperatorPrecedence::kBetween),
              OperatorPrecedence::kBetween);
}

Expr Expr::IsTrue() const {
  return Expr(Extract(OperatorPrecedence::kIs) + " IS TRUE",
              OperatorPrecedence::kIs);
}

Expr Expr::IsFalse() const {
  return Expr(Extract(OperatorPrecedence::kIs) + " IS FALSE",
              OperatorPrecedence::kIs);
}

Expr Expr::IsNull() const {
  return Expr(Extract(OperatorPrecedence::kIs) + " IS NULL",
              OperatorPrecedence::kIs);
}

Expr Expr::IsNotNull() const {
  return Expr(Extract(OperatorPrecedence::kIs) + " IS NOT NULL",
              OperatorPrecedence::kIs);
}

Expr Expr::operator<(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kCompare) + " < " +
                  other.Extract(OperatorPrecedence::kCompare),
              OperatorPrecedence::kCompare);
}

Expr Expr::operator<=(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kCompare) +
                  " <= " + other.Extract(OperatorPrecedence::kCompare),
              OperatorPrecedence::kCompare);
}

Expr Expr::operator>(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kCompare) + " > " +
                  other.Extract(OperatorPrecedence::kCompare),
              OperatorPrecedence::kCompare);
}

Expr Expr::operator>=(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kCompare) +
                  " >= " + other.Extract(OperatorPrecedence::kCompare),
              OperatorPrecedence::kCompare);
}

Expr Expr::operator==(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kCompare) + " = " +
                  other.Extract(OperatorPrecedence::kCompare),
              OperatorPrecedence::kCompare);
}

Expr Expr::operator!=(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kCompare) +
                  " != " + other.Extract(OperatorPrecedence::kCompare),
              OperatorPrecedence::kCompare);
}

Expr Expr::operator||(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kOr) + " OR " +
                  other.Extract(OperatorPrecedence::kOr),
              OperatorPrecedence::kOr);
}

Expr Expr::operator&&(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kAnd) + " AND " +
                  other.Extract(OperatorPrecedence::kAnd),
              OperatorPrecedence::kAnd);
}

Expr Expr::operator+(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kPlus) + " + " +
                  other.Extract(OperatorPrecedence::kPlus),
              OperatorPrecedence::kPlus);
}

Expr Expr::operator-(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kPlus) + " - " +
                  other.Extract(OperatorPrecedence::kPlus),
              OperatorPrecedence::kPlus);
}

Expr Expr::operator*(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kMul) + " * " +
                  other.Extract(OperatorPrecedence::kMul),
              OperatorPrecedence::kMul);
}

Expr Expr::operator/(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kMul) + " / " +
                  other.Extract(OperatorPrecedence::kMul),
              OperatorPrecedence::kMul);
}

Expr Expr::operator%(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kMul) + " % " +
                  other.Extract(OperatorPrecedence::kMul),
              OperatorPrecedence::kMul);
}

Expr Expr::operator!() const {
  return Expr("NOT " + Extract(OperatorPrecedence::kNot),
              OperatorPrecedence::kNot);
}

std::string Expr::Extract(OperatorPrecedence precedence) const {
  if (precedence_ >= precedence)
    return "(" + expr_ + ")";
  return expr_;
}

std::string Expr::ToString() const {
  return Extract(OperatorPrecedence::kExtract);
}

// ---------------------------------------------------------------------------
// CaseBuilder
// ---------------------------------------------------------------------------

CaseBuilder CaseBuilder::When(Expr cond) && {
  if (has_pending_when_)
    throw std::logic_error(
        "iron_query: When() called twice without a matching Then()");
  pending_when_ = cond.ToString();
  has_pending_when_ = true;
  return std::move(*this);
}

CaseBuilder CaseBuilder::Then(Expr result) && {
  if (!has_pending_when_)
    throw std::logic_error("iron_query: Then() called without a "
                           "preceding When()");
  whens_ += "WHEN " + pending_when_ + " THEN " + result.ToString() + " ";
  has_pending_when_ = false;
  return std::move(*this);
}

CaseBuilder CaseBuilder::Else(Expr result) && {
  else_ = result.ToString();
  return std::move(*this);
}

Expr CaseBuilder::End() const {
  if (whens_.empty())
    throw std::logic_error(
        "iron_query: CASE requires at least one When()/Then() pair");

  std::string s = "CASE " + whens_;
  if (!else_.empty())
    s += "ELSE " + else_ + " ";
  s += "END";
  return Expr(std::move(s), OperatorPrecedence::kSymbol);
}

CaseBuilder Case() { return CaseBuilder(); }

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

// ---------------------------------------------------------------------------
// VirtualTable / Table
// ---------------------------------------------------------------------------

std::string VirtualTable::ToStringBracketed() const {
  return "(" + ToString() + ")";
}

Table VirtualTable::As(std::string_view name) const {
  return Table("(" + ToStringBracketed() + " AS " + std::string(name) + ")");
}

VirtualTable::operator Expr() const && {
  return Expr(ToStringBracketed(), OperatorPrecedence::kSymbol);
}

Table::Table(std::string name) : name_(std::move(name)) {}

std::string Table::ToString() const { return name_; }

std::string Table::ToStringBracketed() const {
  // It is a table with name, no need to bracket it
  return ToString();
}

// ---------------------------------------------------------------------------
// SelectExpr
// ---------------------------------------------------------------------------

SelectExpr::SelectExpr(const Table &tbl) : from_(tbl.ToStringBracketed()) {}

SelectExpr SelectExpr::Select(Expr exp) && {
  select_ = exp.ToString();
  return std::move(*this);
}

SelectExpr SelectExpr::Select(std::initializer_list<Expr> exps) && {
  for (const auto &exp : exps) {
    if (!select_.empty())
      select_ += ", ";
    select_ += exp.ToString();
  }
  return std::move(*this);
}

SelectExpr SelectExpr::Where(Expr exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

SelectExpr SelectExpr::OrderBy(std::string_view by) && {
  order_by_ = by;
  return std::move(*this);
}

SelectExpr SelectExpr::OrderBy(std::initializer_list<std::string_view> by) && {
  for (const auto &arg : by) {
    if (!order_by_.empty())
      order_by_ += ", ";
    order_by_ += arg;
  }
  return std::move(*this);
}

SelectExpr SelectExpr::GroupBy(std::string_view by) && {
  group_by_ = by;
  return std::move(*this);
}

SelectExpr SelectExpr::GroupBy(std::initializer_list<std::string_view> by) && {
  for (const auto &arg : by) {
    if (!group_by_.empty())
      group_by_ += ", ";
    group_by_ += arg;
  }
  return std::move(*this);
}

SelectExpr SelectExpr::Having(Expr exp) && {
  having_ = exp.ToString();
  return std::move(*this);
}

SelectExpr SelectExpr::Limit(int limit) && {
  limit_ = std::to_string(limit);
  return std::move(*this);
}

SelectExpr SelectExpr::Offset(int offset) && {
  offset_ = std::to_string(offset);
  return std::move(*this);
}

std::string SelectExpr::ToString() const {
  if (select_.empty())
    throw std::logic_error("iron_query: SELECT clause is not set");
  if (from_.empty())
    throw std::logic_error("iron_query: FROM clause is not set");

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

SelectExpr From(const Table &tbl) { return SelectExpr(tbl); }

// ---------------------------------------------------------------------------
// DeleteFrom
// ---------------------------------------------------------------------------

DeleteFrom::DeleteFrom(const Table &tbl) : from_(tbl.ToStringBracketed()) {}

DeleteFrom DeleteFrom::Where(Expr exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

std::string DeleteFrom::ToString() const {
  if (from_.empty())
    throw std::logic_error("iron_query: FROM clause is not set");

  auto s = "DELETE FROM " + from_;
  if (!where_.empty())
    s += " WHERE " + where_;
  return s;
}

// ---------------------------------------------------------------------------
// InsertInto
// ---------------------------------------------------------------------------

InsertInto::InsertInto(const Table &tbl) : into_(tbl.ToStringBracketed()) {}

InsertInto InsertInto::Columns(std::initializer_list<Expr> cols) && {
  for (const auto &col : cols) {
    if (!columns_.empty())
      columns_ += ", ";
    columns_ += col.ToString();
  }
  return std::move(*this);
}

InsertInto InsertInto::Values(std::initializer_list<Expr> vals) && {
  for (const auto &val : vals) {
    if (!values_.empty())
      values_ += ", ";
    values_ += val.ToString();
  }
  return std::move(*this);
}

std::string InsertInto::ToString() const {
  if (columns_.empty())
    throw std::logic_error("iron_query: no columns to insert into");
  if (values_.empty())
    throw std::logic_error("iron_query: no values to insert");

  return "INSERT INTO " + into_ + " (" + columns_ + ") VALUES (" + values_ +
         ")";
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

Update::Update(const Table &tbl) : table_(tbl.ToStringBracketed()) {}

Update Update::Set(const Expr &column, const Expr &value) && {
  if (!set_.empty())
    set_ += ", ";
  set_ += column.ToString() + " = " + value.ToString();
  return std::move(*this);
}

Update Update::Where(Expr exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

std::string Update::ToString() const {
  if (set_.empty())
    throw std::logic_error("iron_query: SET clause is not set");

  auto s = "UPDATE " + table_ + " SET " + set_;
  if (!where_.empty())
    s += " WHERE " + where_;
  return s;
}

// ---------------------------------------------------------------------------
// JoinKind implementations
// ---------------------------------------------------------------------------

std::string_view Inner::ToString() const { return "INNER"; }
std::string_view Cross::ToString() const { return "CROSS"; }
std::string_view LeftOuter::ToString() const { return "LEFT OUTER"; }
std::string_view RightOuter::ToString() const { return "RIGHT OUTER"; }
std::string_view FullOuter::ToString() const { return "FULL OUTER"; }

// ---------------------------------------------------------------------------
// Join
// ---------------------------------------------------------------------------

Join::Join(const VirtualTable &a, const VirtualTable &b, const JoinKind &kind)
    : a_(a.ToStringBracketed()), b_(b.ToStringBracketed()),
      kind_(kind.ToString()) {}

Join Join::On(Expr exp) && {
  on_ = exp.ToString();
  return std::move(*this);
}

std::string Join::ToString() const {
  auto s = a_ + " " + kind_ + " JOIN " + b_;
  if (!on_.empty())
    s += " ON " + on_;
  return s;
}

// ---------------------------------------------------------------------------
// SetOpKind implementations
// ---------------------------------------------------------------------------

std::string_view Union::ToString() const { return "UNION"; }
std::string_view UnionAll::ToString() const { return "UNION ALL"; }
std::string_view Intersect::ToString() const { return "INTERSECT"; }
std::string_view Except::ToString() const { return "EXCEPT"; }

// ---------------------------------------------------------------------------
// SetOp
// ---------------------------------------------------------------------------

SetOp::SetOp(const VirtualTable &a, const VirtualTable &b,
             const SetOpKind &kind)
    : a_(a.ToStringBracketed()), b_(b.ToStringBracketed()),
      kind_(kind.ToString()) {}

std::string SetOp::ToString() const { return a_ + " " + kind_ + " " + b_; }

// ---------------------------------------------------------------------------
// WithQuery / WithBuilder
// ---------------------------------------------------------------------------

WithQuery::WithQuery(std::string ctes, std::string main)
    : ctes_(std::move(ctes)), main_(std::move(main)) {}

std::string WithQuery::ToString() const {
  return "WITH " + ctes_ + " " + main_;
}

WithBuilder::WithBuilder(std::string name, const VirtualTable &query)
    : ctes_(std::move(name) + " AS " + query.ToStringBracketed()) {}

WithBuilder WithBuilder::With(std::string name, const VirtualTable &query) && {
  ctes_ += ", " + std::move(name) + " AS " + query.ToStringBracketed();
  return std::move(*this);
}

WithQuery WithBuilder::Main(const VirtualTable &query) && {
  return WithQuery(std::move(ctes_), query.ToString());
}

WithBuilder With(std::string name, const VirtualTable &query) {
  return WithBuilder(std::move(name), query);
}

// ---------------------------------------------------------------------------
// Column
// ---------------------------------------------------------------------------

Column::operator Expr() const { return Expr(name); }

Expr Column::operator<(const Expr &other) const { return Expr(*this) < other; }

Expr Column::operator<=(const Expr &other) const {
  return Expr(*this) <= other;
}

Expr Column::operator>(const Expr &other) const { return Expr(*this) > other; }

Expr Column::operator>=(const Expr &other) const {
  return Expr(*this) >= other;
}

Expr Column::operator==(const Expr &other) const {
  return Expr(*this) == other;
}

Expr Column::operator!=(const Expr &other) const {
  return Expr(*this) != other;
}

// ---------------------------------------------------------------------------
// TableWithColumns
// ---------------------------------------------------------------------------

TableWithColumns::TableWithColumns(std::string name,
                                   std::vector<Column> columns)
    : Table(std::move(name)), columns_(columns) {}

TableWithColumns::TableWithColumns(std::string name,
                                   std::initializer_list<Column> columns)
    : TableWithColumns(std::move(name), std::vector(columns)) {}

Expr TableWithColumns::SelectArgAll() const {
  std::string s;
  for (const auto &col : columns_) {
    if (!s.empty())
      s += ", ";
    s += col.name;
  }
  return Expr(s);
}

// ---------------------------------------------------------------------------
// TableAlias
// ---------------------------------------------------------------------------

TableAlias::TableAlias(std::string_view alias) : alias_(alias) {}

std::string TableAlias::Dot(const std::string &column) const {
  return alias_ + "." + column;
}

std::string TableAlias::Dot(const Column &column) const {
  return Dot(column.name);
}

} // namespace iron_query
