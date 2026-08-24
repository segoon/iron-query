#include <iron_query/iron_query.hpp>

namespace iron_query {

// ---------------------------------------------------------------------------
// Expr
// ---------------------------------------------------------------------------

Expr::Expr(std::string s)
    : expr_(std::move(s)), precedence_(OperatorPrecedence::kSymbol) {}

Expr::Expr(int i) : Expr(std::to_string(i)) {}

Expr::Expr(std::string expr, OperatorPrecedence precedence)
    : expr_(std::move(expr)), precedence_(precedence) {}

Expr Expr::FromRaw(std::string s) { return Expr(std::move(s)); }

Expr Expr::FromRaw(std::string s, OperatorPrecedence precedence) {
  return Expr(std::move(s), precedence);
}

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

Expr Expr::CallRaw(const std::string &name, std::initializer_list<Expr> args) {
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

Condition Expr::Exists(const VirtualTable &subquery) {
  return Condition("EXISTS " + subquery.ToStringBracketed(),
                   OperatorPrecedence::kSymbol);
}

Condition Expr::NotExists(const VirtualTable &subquery) {
  return Condition("NOT EXISTS " + subquery.ToStringBracketed(),
                   OperatorPrecedence::kSymbol);
}

Expr Expr::Count(const Expr &arg) { return CallRaw("COUNT", {arg}); }

Expr Expr::CountAll() { return Expr("COUNT(*)"); }

Expr Expr::Sum(const Expr &arg) { return CallRaw("SUM", {arg}); }

Expr Expr::Avg(const Expr &arg) { return CallRaw("AVG", {arg}); }

Expr Expr::Min(const Expr &arg) { return CallRaw("MIN", {arg}); }

Expr Expr::Max(const Expr &arg) { return CallRaw("MAX", {arg}); }

Expr Expr::Dot(const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kDot) + "." +
                  other.Extract(OperatorPrecedence::kDot),
              OperatorPrecedence::kDot);
}

Expr Expr::CastRaw(const std::string &type) const {
  return Expr("CAST (" + Extract(OperatorPrecedence::kTypecast) + " AS " +
                  type + ")",
              OperatorPrecedence::kTypecast);
}

Expr Expr::CollateRaw(const std::string &collation) const {
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

Condition Expr::Between(const Expr &a, const Expr &b) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " BETWEEN " +
                       a.Extract(OperatorPrecedence::kBetween) + " AND " +
                       b.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::NotBetween(const Expr &a, const Expr &b) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " NOT BETWEEN " +
                       a.Extract(OperatorPrecedence::kBetween) + " AND " +
                       b.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::Like(const Expr &a) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " LIKE " +
                       a.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::NotLike(const Expr &a) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " NOT LIKE " +
                       a.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::In(const Expr &a) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " IN " +
                       a.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::NotIn(const Expr &a) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " NOT IN " +
                       a.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::IsTrue() const {
  return Condition(Extract(OperatorPrecedence::kIs) + " IS TRUE",
                   OperatorPrecedence::kIs);
}

Condition Expr::IsFalse() const {
  return Condition(Extract(OperatorPrecedence::kIs) + " IS FALSE",
                   OperatorPrecedence::kIs);
}

Condition Expr::IsNull() const {
  return Condition(Extract(OperatorPrecedence::kIs) + " IS NULL",
                   OperatorPrecedence::kIs);
}

Condition Expr::IsNotNull() const {
  return Condition(Extract(OperatorPrecedence::kIs) + " IS NOT NULL",
                   OperatorPrecedence::kIs);
}

Condition Expr::operator<(const Expr &other) const {
  return Condition(Extract(OperatorPrecedence::kCompare) + " < " +
                       other.Extract(OperatorPrecedence::kCompare),
                   OperatorPrecedence::kCompare);
}

Condition Expr::operator<=(const Expr &other) const {
  return Condition(Extract(OperatorPrecedence::kCompare) +
                       " <= " + other.Extract(OperatorPrecedence::kCompare),
                   OperatorPrecedence::kCompare);
}

Condition Expr::operator>(const Expr &other) const {
  return Condition(Extract(OperatorPrecedence::kCompare) + " > " +
                       other.Extract(OperatorPrecedence::kCompare),
                   OperatorPrecedence::kCompare);
}

Condition Expr::operator>=(const Expr &other) const {
  return Condition(Extract(OperatorPrecedence::kCompare) +
                       " >= " + other.Extract(OperatorPrecedence::kCompare),
                   OperatorPrecedence::kCompare);
}

Condition Expr::operator==(const Expr &other) const {
  return Condition(Extract(OperatorPrecedence::kCompare) + " = " +
                       other.Extract(OperatorPrecedence::kCompare),
                   OperatorPrecedence::kCompare);
}

Condition Expr::operator!=(const Expr &other) const {
  return Condition(Extract(OperatorPrecedence::kCompare) +
                       " != " + other.Extract(OperatorPrecedence::kCompare),
                   OperatorPrecedence::kCompare);
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

std::string Expr::Extract(OperatorPrecedence precedence) const {
  if (precedence_ >= precedence)
    return "(" + expr_ + ")";
  return expr_;
}

std::string Expr::ToString() const {
  return Extract(OperatorPrecedence::kExtract);
}

// ---------------------------------------------------------------------------
// Condition
// ---------------------------------------------------------------------------

Condition::Condition(std::string s, OperatorPrecedence precedence)
    : expr_(std::move(s)), precedence_(precedence) {}

Condition Condition::FromRaw(std::string s) {
  return Condition(std::move(s), OperatorPrecedence::kSymbol);
}

Condition Condition::operator&&(const Condition &other) const {
  return Condition(Extract(OperatorPrecedence::kAnd) + " AND " +
                       other.Extract(OperatorPrecedence::kAnd),
                   OperatorPrecedence::kAnd);
}

Condition Condition::operator||(const Condition &other) const {
  return Condition(Extract(OperatorPrecedence::kOr) + " OR " +
                       other.Extract(OperatorPrecedence::kOr),
                   OperatorPrecedence::kOr);
}

Condition Condition::operator!() const {
  return Condition("NOT " + Extract(OperatorPrecedence::kNot),
                   OperatorPrecedence::kNot);
}

std::string Condition::Extract(OperatorPrecedence precedence) const {
  if (precedence_ >= precedence)
    return "(" + expr_ + ")";
  return expr_;
}

std::string Condition::ToString() const {
  return Extract(OperatorPrecedence::kExtract);
}

Condition::operator Expr() const && {
  return Expr::FromRaw(expr_, precedence_);
}

// ---------------------------------------------------------------------------
// CaseBuilder
// ---------------------------------------------------------------------------

CaseBuilder CaseBuilder::When(Condition cond) && {
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
  return Expr::FromRaw(std::move(s), OperatorPrecedence::kSymbol);
}

CaseBuilder Case() { return CaseBuilder(); }

const Expr _1 = Expr::FromRaw("$1");
const Expr _2 = Expr::FromRaw("$2");
const Expr _3 = Expr::FromRaw("$3");
const Expr _4 = Expr::FromRaw("$4");
const Expr _5 = Expr::FromRaw("$5");
const Expr _6 = Expr::FromRaw("$6");
const Expr _7 = Expr::FromRaw("$7");
const Expr _8 = Expr::FromRaw("$8");
const Expr _9 = Expr::FromRaw("$9");
const Expr _10 = Expr::FromRaw("$10");

// ---------------------------------------------------------------------------
// VirtualTable / Table
// ---------------------------------------------------------------------------

std::string VirtualTable::ToStringFormatted() const { return ToString(); }

std::string VirtualTable::ToStringBracketed() const {
  return "(" + ToString() + ")";
}

Table VirtualTable::As(std::string_view name) const {
  return Table::FromRaw("(" + ToStringBracketed() + " AS " + std::string(name) +
                        ")");
}

VirtualTable::operator Expr() const && {
  return Expr::FromRaw(ToStringBracketed(), OperatorPrecedence::kSymbol);
}

Table::Table(std::string name) : name_(std::move(name)) {}

Table Table::FromRaw(std::string name) { return Table(std::move(name)); }

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
  select_ = {exp.ToString()};
  return std::move(*this);
}

SelectExpr SelectExpr::Select(std::initializer_list<Expr> exps) && {
  for (const auto &exp : exps)
    select_.push_back(exp.ToString());
  return std::move(*this);
}

SelectExpr SelectExpr::Where(Condition exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

SelectExpr SelectExpr::OrderByRaw(std::string_view by) && {
  order_by_ = {std::string(by)};
  return std::move(*this);
}

SelectExpr
SelectExpr::OrderByRaw(std::initializer_list<std::string_view> by) && {
  for (const auto &arg : by)
    order_by_.emplace_back(arg);
  return std::move(*this);
}

SelectExpr SelectExpr::GroupByRaw(std::string_view by) && {
  group_by_ = {std::string(by)};
  return std::move(*this);
}

SelectExpr
SelectExpr::GroupByRaw(std::initializer_list<std::string_view> by) && {
  for (const auto &arg : by)
    group_by_.emplace_back(arg);
  return std::move(*this);
}

SelectExpr SelectExpr::Having(Condition exp) && {
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

namespace {

std::string JoinCsv(const std::vector<std::string> &items) {
  std::string s;
  for (const auto &item : items) {
    if (!s.empty())
      s += ", ";
    s += item;
  }
  return s;
}

std::string JoinCsvIndented(const std::vector<std::string> &items) {
  std::string s;
  for (const auto &item : items) {
    if (!s.empty())
      s += ",\n";
    s += "    " + item;
  }
  return s;
}

} // namespace

void SelectExpr::EnsureValid() const {
  if (select_.empty())
    throw std::logic_error("iron_query: SELECT clause is not set");
  if (from_.empty())
    throw std::logic_error("iron_query: FROM clause is not set");
}

std::string SelectExpr::ToString() const {
  EnsureValid();

  auto s = "SELECT " + JoinCsv(select_) + " FROM " + from_;
  if (!where_.empty())
    s += " WHERE " + where_;
  if (!group_by_.empty())
    s += " GROUP BY " + JoinCsv(group_by_);
  if (!having_.empty())
    s += " HAVING " + having_;
  if (!order_by_.empty())
    s += " ORDER BY " + JoinCsv(order_by_);
  if (!limit_.empty())
    s += " LIMIT " + limit_;
  if (!offset_.empty())
    s += " OFFSET " + offset_;
  return s;
}

std::string SelectExpr::ToStringFormatted() const {
  EnsureValid();

  auto s = "SELECT\n" + JoinCsvIndented(select_) + "\nFROM\n    " + from_;
  if (!where_.empty())
    s += "\nWHERE\n    " + where_;
  if (!group_by_.empty())
    s += "\nGROUP BY\n" + JoinCsvIndented(group_by_);
  if (!having_.empty())
    s += "\nHAVING\n    " + having_;
  if (!order_by_.empty())
    s += "\nORDER BY\n" + JoinCsvIndented(order_by_);
  if (!limit_.empty())
    s += "\nLIMIT\n    " + limit_;
  if (!offset_.empty())
    s += "\nOFFSET\n    " + offset_;
  return s;
}

SelectExpr From(const Table &tbl) { return SelectExpr(tbl); }

// ---------------------------------------------------------------------------
// DeleteFrom
// ---------------------------------------------------------------------------

DeleteFrom::DeleteFrom(const Table &tbl) : from_(tbl.ToStringBracketed()) {}

DeleteFrom DeleteFrom::Where(Condition exp) && {
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

Update Update::Where(Condition exp) && {
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

Join Join::On(Condition exp) && {
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

WithBuilder WithBuilder::WithRaw(std::string name,
                                 const VirtualTable &query) && {
  ctes_ += ", " + std::move(name) + " AS " + query.ToStringBracketed();
  return std::move(*this);
}

WithQuery WithBuilder::Main(const VirtualTable &query) && {
  return WithQuery(std::move(ctes_), query.ToString());
}

WithBuilder WithRaw(std::string name, const VirtualTable &query) {
  return WithBuilder(std::move(name), query);
}

// ---------------------------------------------------------------------------
// Column
// ---------------------------------------------------------------------------

Column::operator Expr() const { return Expr::FromRaw(name); }

Condition Column::operator<(const Expr &other) const {
  return Expr(*this) < other;
}

Condition Column::operator<=(const Expr &other) const {
  return Expr(*this) <= other;
}

Condition Column::operator>(const Expr &other) const {
  return Expr(*this) > other;
}

Condition Column::operator>=(const Expr &other) const {
  return Expr(*this) >= other;
}

Condition Column::operator==(const Expr &other) const {
  return Expr(*this) == other;
}

Condition Column::operator!=(const Expr &other) const {
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

TableWithColumns TableWithColumns::FromRaw(std::string name,
                                           std::vector<Column> columns) {
  return TableWithColumns(std::move(name), std::move(columns));
}

TableWithColumns
TableWithColumns::FromRaw(std::string name,
                          std::initializer_list<Column> columns) {
  return TableWithColumns(std::move(name), columns);
}

Expr TableWithColumns::SelectArgAll() const {
  std::string s;
  for (const auto &col : columns_) {
    if (!s.empty())
      s += ", ";
    s += col.name;
  }
  return Expr::FromRaw(s);
}

// ---------------------------------------------------------------------------
// TableAlias
// ---------------------------------------------------------------------------

TableAlias::TableAlias(std::string_view alias) : alias_(alias) {}

TableAlias TableAlias::FromRaw(std::string_view alias) {
  return TableAlias(alias);
}

std::string TableAlias::Dot(const std::string &column) const {
  return alias_ + "." + column;
}

std::string TableAlias::Dot(const Column &column) const {
  return Dot(column.name);
}

} // namespace iron_query
