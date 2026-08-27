#include "impl/identifier.hpp"
#include "impl/render.hpp"
#include <array>
#include <charconv>
#include <cmath>
#include <iron_query/collation.hpp>
#include <iron_query/condition.hpp>
#include <iron_query/expr.hpp>
#include <iron_query/select_item.hpp>
#include <iron_query/virtual_table.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace iron_query {

Expr::Expr(std::string s)
    : expr_(std::move(s)), precedence_(OperatorPrecedence::kSymbol) {}

Expr::Expr(std::string expr, OperatorPrecedence precedence)
    : expr_(std::move(expr)), precedence_(precedence) {}

Expr Expr::FromInteger(long long value) { return Expr(std::to_string(value)); }

// Separate from the signed overload: casting an unsigned 64-bit value to
// long long would wrap it.
Expr Expr::FromInteger(unsigned long long value) {
  return Expr(std::to_string(value));
}

Expr Expr::FromDouble(double value) {
  if (!std::isfinite(value))
    throw std::invalid_argument(
        "iron_query: NaN and infinity have no plain SQL literal");

  // std::to_string would round to 6 fractional digits; to_chars gives the
  // shortest representation that reads back as the same double, and is
  // locale-independent.
  std::array<char, 32> buf{};
  auto [end, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
  if (ec != std::errc())
    throw std::invalid_argument("iron_query: cannot render double literal");

  std::string s(buf.data(), end);
  // Keep the value typed as numeric: a bare "1" would be an integer literal.
  if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
    s += ".0";
  return Expr(std::move(s));
}

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

Expr Expr::Null() { return Expr("NULL", OperatorPrecedence::kSymbol); }

Expr Expr::Bool(bool value) {
  return Expr(value ? "TRUE" : "FALSE", OperatorPrecedence::kSymbol);
}

Expr Expr::Call(const std::string &name, std::initializer_list<Expr> args) {
  impl::ValidateIdentifier(name);
  auto s = name + "(";
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

namespace {

// COALESCE/GREATEST/LEAST render like ordinary function calls, but SQL
// requires at least one argument, unlike a real function call.
void EnsureNonEmptyArgs(std::initializer_list<Expr> args, const char *name) {
  if (args.size() == 0)
    throw std::invalid_argument(std::string("iron_query: ") + name +
                                "() needs at least one argument");
}

} // namespace

Expr Expr::Coalesce(std::initializer_list<Expr> args) {
  EnsureNonEmptyArgs(args, "COALESCE");
  return Call("COALESCE", args);
}

Expr Expr::NullIf(const Expr &a, const Expr &b) {
  return Call("NULLIF", {a, b});
}

Expr Expr::Greatest(std::initializer_list<Expr> args) {
  EnsureNonEmptyArgs(args, "GREATEST");
  return Call("GREATEST", args);
}

Expr Expr::Least(std::initializer_list<Expr> args) {
  EnsureNonEmptyArgs(args, "LEAST");
  return Call("LEAST", args);
}

Condition Expr::Exists(const VirtualTable &subquery) {
  return Condition("EXISTS " + subquery.ToStringBracketed(),
                   OperatorPrecedence::kSymbol);
}

Condition Expr::NotExists(const VirtualTable &subquery) {
  return Condition("NOT EXISTS " + subquery.ToStringBracketed(),
                   OperatorPrecedence::kSymbol);
}

Expr Expr::Count(const Expr &arg) { return Call("COUNT", {arg}); }

Expr Expr::CountAll() { return Expr(std::string("COUNT(*)")); }

Expr Expr::CountDistinct(const Expr &arg) {
  return Expr("COUNT(DISTINCT " + arg.ToString() + ")");
}

Expr Expr::Sum(const Expr &arg) { return Call("SUM", {arg}); }

Expr Expr::Avg(const Expr &arg) { return Call("AVG", {arg}); }

Expr Expr::Min(const Expr &arg) { return Call("MIN", {arg}); }

Expr Expr::Max(const Expr &arg) { return Call("MAX", {arg}); }

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

Expr Expr::Collate(const Collation &collation) const {
  return Expr(Extract(OperatorPrecedence::kCollate) + " COLLATE " +
                  collation.ToString(),
              OperatorPrecedence::kCollate);
}

Expr Expr::operator[](const Expr &other) const {
  return Expr(Extract(OperatorPrecedence::kIndex) + "[" + other.ToString() +
                  "]",
              OperatorPrecedence::kIndex);
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

Condition Expr::ILike(const Expr &a) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " ILIKE " +
                       a.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::NotILike(const Expr &a) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " NOT ILIKE " +
                       a.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::SimilarTo(const Expr &a) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " SIMILAR TO " +
                       a.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

Condition Expr::NotSimilarTo(const Expr &a) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " NOT SIMILAR TO " +
                       a.Extract(OperatorPrecedence::kBetween),
                   OperatorPrecedence::kBetween);
}

namespace {

// The right-hand side of IN/ANY/ALL is always parenthesized by the grammar
// itself, so it is built here rather than via Extract().
std::string RenderValueList(std::initializer_list<Expr> values) {
  if (values.size() == 0)
    throw std::invalid_argument("iron_query: IN () needs at least one value");

  std::vector<std::string> rendered;
  rendered.reserve(values.size());
  for (const auto &value : values)
    rendered.push_back(value.ToString());
  return "(" + impl::JoinCsv(rendered) + ")";
}

} // namespace

Condition Expr::In(std::initializer_list<Expr> values) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " IN " +
                       RenderValueList(values),
                   OperatorPrecedence::kBetween);
}

Condition Expr::In(const VirtualTable &subquery) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " IN " +
                       subquery.ToStringBracketed(),
                   OperatorPrecedence::kBetween);
}

Condition Expr::NotIn(std::initializer_list<Expr> values) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " NOT IN " +
                       RenderValueList(values),
                   OperatorPrecedence::kBetween);
}

Condition Expr::NotIn(const VirtualTable &subquery) const {
  return Condition(Extract(OperatorPrecedence::kBetween) + " NOT IN " +
                       subquery.ToStringBracketed(),
                   OperatorPrecedence::kBetween);
}

Condition Expr::EqAny(const Expr &array) const {
  return Condition(Extract(OperatorPrecedence::kCompare) + " = ANY (" +
                       array.ToString() + ")",
                   OperatorPrecedence::kCompare);
}

Condition Expr::EqAny(const VirtualTable &subquery) const {
  return Condition(Extract(OperatorPrecedence::kCompare) + " = ANY " +
                       subquery.ToStringBracketed(),
                   OperatorPrecedence::kCompare);
}

Condition Expr::NeAll(const Expr &array) const {
  return Condition(Extract(OperatorPrecedence::kCompare) + " <> ALL (" +
                       array.ToString() + ")",
                   OperatorPrecedence::kCompare);
}

Condition Expr::NeAll(const VirtualTable &subquery) const {
  return Condition(Extract(OperatorPrecedence::kCompare) + " <> ALL " +
                       subquery.ToStringBracketed(),
                   OperatorPrecedence::kCompare);
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

Condition Expr::IsDistinctFrom(const Expr &other) const {
  return Condition(Extract(OperatorPrecedence::kIs) + " IS DISTINCT FROM " +
                       other.Extract(OperatorPrecedence::kIs),
                   OperatorPrecedence::kIs);
}

Condition Expr::IsNotDistinctFrom(const Expr &other) const {
  return Condition(Extract(OperatorPrecedence::kIs) + " IS NOT DISTINCT FROM " +
                       other.Extract(OperatorPrecedence::kIs),
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

SelectItem Expr::As(std::string_view name) const {
  // Not impl::ValidateIdentifier(): a column alias cannot be dot-qualified.
  if (!impl::IsPlainIdentifier(name))
    throw std::invalid_argument("iron_query: invalid column alias: " +
                                std::string(name));
  return SelectItem(ToString() + " AS " + std::string(name));
}

std::string Expr::Extract(OperatorPrecedence precedence) const {
  if (precedence_ >= precedence)
    return "(" + expr_ + ")";
  return expr_;
}

std::string Expr::ToString() const {
  return Extract(OperatorPrecedence::kExtract);
}

} // namespace iron_query
