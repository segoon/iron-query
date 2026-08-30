#include "impl/identifier.hpp"
#include "impl/node.hpp"
#include <array>
#include <charconv>
#include <cmath>
#include <cstring>
#include <iron_query/collation.hpp>
#include <iron_query/condition.hpp>
#include <iron_query/exception.hpp>
#include <iron_query/expr.hpp>
#include <iron_query/select_item.hpp>
#include <iron_query/virtual_table.hpp>
#include <string>
#include <utility>
#include <vector>

namespace iron_query {

Expr::Expr(std::string s)
    : node_(impl::MakeLeaf(std::move(s), OperatorPrecedence::kSymbol)) {}

Expr::Expr(std::string expr, OperatorPrecedence precedence)
    : node_(impl::MakeLeaf(std::move(expr), precedence)) {}

Expr::Expr(std::shared_ptr<const impl::Node> node) : node_(std::move(node)) {}

Expr Expr::FromInteger(long long value) { return Expr(std::to_string(value)); }

// Separate from the signed overload: casting an unsigned 64-bit value to
// long long would wrap it.
Expr Expr::FromInteger(unsigned long long value) {
  return Expr(std::to_string(value));
}

Expr Expr::FromDouble(double value) {
  if (!std::isfinite(value))
    throw InvalidLiteral(std::to_string(value) +
                         ": NaN and infinity have no plain SQL literal");

  // std::to_string would round to 6 fractional digits; to_chars gives the
  // shortest representation that reads back as the same double, and is
  // locale-independent.
  std::array<char, 32> buf{};
  auto [end, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
  if (ec != std::errc())
    throw InvalidLiteral(std::to_string(value) +
                         ": cannot render double literal");

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
    throw InvalidLiteral("string literal must not contain NUL bytes");

  std::string escaped;
  escaped.reserve(value.size() + 2);
  escaped += '\'';
  for (char c : value) {
    if (c == '\'')
      escaped += '\'';
    escaped += c;
  }
  escaped += '\'';
  return Expr(std::move(escaped), OperatorPrecedence::kSymbol);
}

Expr Expr::Ident(const std::string &name) {
  if (name.find('\0') != std::string::npos)
    throw InvalidIdentifier("<identifier containing a NUL byte>");

  std::string escaped;
  escaped.reserve(name.size() + 2);
  escaped += '"';
  for (char c : name) {
    if (c == '"')
      escaped += '"';
    escaped += c;
  }
  escaped += '"';
  return Expr(std::move(escaped), OperatorPrecedence::kSymbol);
}

Expr Expr::Null() { return Expr("NULL", OperatorPrecedence::kSymbol); }

Expr Expr::Bool(bool value) {
  return Expr(value ? "TRUE" : "FALSE", OperatorPrecedence::kSymbol);
}

namespace {

// COALESCE/GREATEST/LEAST render like ordinary function calls, but SQL
// requires at least one argument, unlike a real function call.
void EnsureNonEmptyArgs(std::initializer_list<Expr> args, const char *name) {
  if (args.size() == 0)
    throw InvalidArgument(std::string(name) + "() needs at least one argument");
}

} // namespace

std::shared_ptr<const impl::Node>
Expr::BuildCallNode(const std::string &name, const char *args_prefix,
                    std::initializer_list<Expr> args) {
  std::vector<impl::Part> parts;
  parts.reserve(args.size() * 2 + 1);
  parts.emplace_back(name + "(" + args_prefix);
  bool first = true;
  for (const auto &arg : args) {
    if (!first)
      parts.emplace_back(std::string(", "));
    parts.emplace_back(impl::ChildRef{arg.node_, OperatorPrecedence::kExtract});
    first = false;
  }
  parts.emplace_back(std::string(")"));
  return impl::MakeNode(OperatorPrecedence::kSymbol, std::move(parts));
}

Expr Expr::Call(const std::string &name, std::initializer_list<Expr> args) {
  impl::ValidateIdentifier(name);
  return Expr(BuildCallNode(name, "", args));
}

Expr Expr::CallDistinct(const std::string &name,
                        std::initializer_list<Expr> args) {
  impl::ValidateIdentifier(name);
  EnsureNonEmptyArgs(args, name.c_str());
  return Expr(BuildCallNode(name, "DISTINCT ", args));
}

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

Expr Expr::PrefixOp(const std::string &op, const Expr &operand,
                    OperatorPrecedence precedence) {
  impl::ValidateOperatorName(op);
  return Expr(impl::MakeNode(
      precedence, {op + " ", impl::ChildRef{operand.node_, precedence}}));
}

Expr Expr::Count(const Expr &arg) { return Call("COUNT", {arg}); }

Expr Expr::CountAll() { return Expr(std::string("COUNT(*)")); }

Expr Expr::CountDistinct(const Expr &arg) {
  return CallDistinct("COUNT", {arg});
}

Expr Expr::Sum(const Expr &arg) { return Call("SUM", {arg}); }

Expr Expr::Avg(const Expr &arg) { return Call("AVG", {arg}); }

Expr Expr::Min(const Expr &arg) { return Call("MIN", {arg}); }

Expr Expr::Max(const Expr &arg) { return Call("MAX", {arg}); }

Expr Expr::Dot(const Expr &other) const {
  return Expr(impl::MakeNode(
      OperatorPrecedence::kDot,
      {impl::ChildRef{node_, OperatorPrecedence::kDot}, std::string("."),
       impl::ChildRef{other.node_, OperatorPrecedence::kDot}}));
}

Expr Expr::CastRaw(const std::string &type) const {
  return Expr(
      impl::MakeNode(OperatorPrecedence::kTypecast,
                     {std::string("CAST ("),
                      impl::ChildRef{node_, OperatorPrecedence::kTypecast},
                      " AS " + type + ")"}));
}

Expr Expr::Collate(const Collation &collation) const {
  return Expr(
      impl::MakeNode(OperatorPrecedence::kCollate,
                     {impl::ChildRef{node_, OperatorPrecedence::kCollate},
                      " COLLATE " + collation.ToString()}));
}

Expr Expr::operator[](const Expr &other) const {
  return Expr(impl::MakeNode(
      OperatorPrecedence::kIndex,
      {impl::ChildRef{node_, OperatorPrecedence::kIndex}, std::string("["),
       impl::ChildRef{other.node_, OperatorPrecedence::kExtract},
       std::string("]")}));
}

Expr Expr::operator^(const Expr &other) const {
  return BinaryOp("^", other, OperatorPrecedence::kExp);
}

Expr Expr::BinaryOp(const std::string &op, const Expr &other,
                    OperatorPrecedence precedence) const {
  impl::ValidateOperatorName(op);
  return Expr(impl::MakeNode(precedence,
                             {impl::ChildRef{node_, precedence}, " " + op + " ",
                              impl::ChildRef{other.node_, precedence}}));
}

Condition Expr::Between(const Expr &a, const Expr &b) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      std::string(" BETWEEN "),
                      impl::ChildRef{a.node_, OperatorPrecedence::kBetween},
                      std::string(" AND "),
                      impl::ChildRef{b.node_, OperatorPrecedence::kBetween}}));
}

Condition Expr::NotBetween(const Expr &a, const Expr &b) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      std::string(" NOT BETWEEN "),
                      impl::ChildRef{a.node_, OperatorPrecedence::kBetween},
                      std::string(" AND "),
                      impl::ChildRef{b.node_, OperatorPrecedence::kBetween}}));
}

Condition Expr::Like(const Expr &a) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      std::string(" LIKE "),
                      impl::ChildRef{a.node_, OperatorPrecedence::kBetween}}));
}

Condition Expr::NotLike(const Expr &a) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      std::string(" NOT LIKE "),
                      impl::ChildRef{a.node_, OperatorPrecedence::kBetween}}));
}

Condition Expr::ILike(const Expr &a) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      std::string(" ILIKE "),
                      impl::ChildRef{a.node_, OperatorPrecedence::kBetween}}));
}

Condition Expr::NotILike(const Expr &a) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      std::string(" NOT ILIKE "),
                      impl::ChildRef{a.node_, OperatorPrecedence::kBetween}}));
}

Condition Expr::SimilarTo(const Expr &a) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      std::string(" SIMILAR TO "),
                      impl::ChildRef{a.node_, OperatorPrecedence::kBetween}}));
}

Condition Expr::NotSimilarTo(const Expr &a) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      std::string(" NOT SIMILAR TO "),
                      impl::ChildRef{a.node_, OperatorPrecedence::kBetween}}));
}

// The right-hand side of IN/ANY/ALL is always parenthesized by the grammar
// itself, so its elements are embedded at kExtract (never separately
// bracketed) rather than via the outer BETWEEN-level context.
std::shared_ptr<const impl::Node>
Expr::BuildInListNode(const Expr &self, const char *connector,
                      std::initializer_list<Expr> values) {
  if (values.size() == 0)
    throw InvalidArgument("IN () needs at least one value");

  std::vector<impl::Part> parts{
      impl::ChildRef{self.node_, OperatorPrecedence::kBetween},
      std::string(connector), std::string("(")};
  bool first = true;
  for (const auto &value : values) {
    if (!first)
      parts.emplace_back(std::string(", "));
    parts.emplace_back(
        impl::ChildRef{value.node_, OperatorPrecedence::kExtract});
    first = false;
  }
  parts.emplace_back(std::string(")"));
  return impl::MakeNode(OperatorPrecedence::kBetween, std::move(parts));
}

Condition Expr::In(std::initializer_list<Expr> values) const {
  return Condition(BuildInListNode(*this, " IN ", values));
}

Condition Expr::In(const VirtualTable &subquery) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      " IN " + subquery.ToStringBracketed()}));
}

Condition Expr::NotIn(std::initializer_list<Expr> values) const {
  return Condition(BuildInListNode(*this, " NOT IN ", values));
}

Condition Expr::NotIn(const VirtualTable &subquery) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kBetween,
                     {impl::ChildRef{node_, OperatorPrecedence::kBetween},
                      " NOT IN " + subquery.ToStringBracketed()}));
}

Condition Expr::EqAny(const Expr &array) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kCompare,
                     {impl::ChildRef{node_, OperatorPrecedence::kCompare},
                      std::string(" = ANY ("),
                      impl::ChildRef{array.node_, OperatorPrecedence::kExtract},
                      std::string(")")}));
}

Condition Expr::EqAny(const VirtualTable &subquery) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kCompare,
                     {impl::ChildRef{node_, OperatorPrecedence::kCompare},
                      " = ANY " + subquery.ToStringBracketed()}));
}

Condition Expr::NeAll(const Expr &array) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kCompare,
                     {impl::ChildRef{node_, OperatorPrecedence::kCompare},
                      std::string(" <> ALL ("),
                      impl::ChildRef{array.node_, OperatorPrecedence::kExtract},
                      std::string(")")}));
}

Condition Expr::NeAll(const VirtualTable &subquery) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kCompare,
                     {impl::ChildRef{node_, OperatorPrecedence::kCompare},
                      " <> ALL " + subquery.ToStringBracketed()}));
}

Condition Expr::IsTrue() const {
  return Condition(impl::MakeNode(
      OperatorPrecedence::kIs, {impl::ChildRef{node_, OperatorPrecedence::kIs},
                                std::string(" IS TRUE")}));
}

Condition Expr::IsFalse() const {
  return Condition(impl::MakeNode(
      OperatorPrecedence::kIs, {impl::ChildRef{node_, OperatorPrecedence::kIs},
                                std::string(" IS FALSE")}));
}

Condition Expr::IsNull() const {
  return Condition(impl::MakeNode(
      OperatorPrecedence::kIs, {impl::ChildRef{node_, OperatorPrecedence::kIs},
                                std::string(" IS NULL")}));
}

Condition Expr::IsNotNull() const {
  return Condition(impl::MakeNode(
      OperatorPrecedence::kIs, {impl::ChildRef{node_, OperatorPrecedence::kIs},
                                std::string(" IS NOT NULL")}));
}

Condition Expr::IsDistinctFrom(const Expr &other) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kIs,
                     {impl::ChildRef{node_, OperatorPrecedence::kIs},
                      std::string(" IS DISTINCT FROM "),
                      impl::ChildRef{other.node_, OperatorPrecedence::kIs}}));
}

Condition Expr::IsNotDistinctFrom(const Expr &other) const {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kIs,
                     {impl::ChildRef{node_, OperatorPrecedence::kIs},
                      std::string(" IS NOT DISTINCT FROM "),
                      impl::ChildRef{other.node_, OperatorPrecedence::kIs}}));
}

Condition Expr::CompareOp(const Expr &a, const char *op, const Expr &b) {
  return Condition(
      impl::MakeNode(OperatorPrecedence::kCompare,
                     {impl::ChildRef{a.node_, OperatorPrecedence::kCompare},
                      std::string(" ") + op + " ",
                      impl::ChildRef{b.node_, OperatorPrecedence::kCompare}}));
}

Condition operator<(const Expr &a, const Expr &b) {
  return Expr::CompareOp(a, "<", b);
}

Condition operator<=(const Expr &a, const Expr &b) {
  return Expr::CompareOp(a, "<=", b);
}

Condition operator>(const Expr &a, const Expr &b) {
  return Expr::CompareOp(a, ">", b);
}

Condition operator>=(const Expr &a, const Expr &b) {
  return Expr::CompareOp(a, ">=", b);
}

Condition operator==(const Expr &a, const Expr &b) {
  return Expr::CompareOp(a, "=", b);
}

Condition operator!=(const Expr &a, const Expr &b) {
  return Expr::CompareOp(a, "!=", b);
}

Expr Expr::operator+(const Expr &other) const {
  return BinaryOp("+", other, OperatorPrecedence::kPlus);
}

Expr Expr::operator-(const Expr &other) const {
  return BinaryOp("-", other, OperatorPrecedence::kPlus);
}

Expr Expr::operator*(const Expr &other) const {
  return BinaryOp("*", other, OperatorPrecedence::kMul);
}

Expr Expr::operator/(const Expr &other) const {
  return BinaryOp("/", other, OperatorPrecedence::kMul);
}

Expr Expr::operator%(const Expr &other) const {
  return BinaryOp("%", other, OperatorPrecedence::kMul);
}

Expr Expr::operator-() const {
  return Expr(
      impl::MakeNode(OperatorPrecedence::kUnaryPlus,
                     {std::string("-"),
                      impl::ChildRef{node_, OperatorPrecedence::kUnaryPlus}}));
}

Expr Expr::operator!() const {
  return Expr(impl::MakeNode(
      OperatorPrecedence::kNot,
      {std::string("NOT "), impl::ChildRef{node_, OperatorPrecedence::kNot}}));
}

Expr Expr::Concat(const Expr &other) const {
  return BinaryOp("||", other, OperatorPrecedence::kAnyOther);
}

SelectItem Expr::As(std::string_view name) const {
  // Not impl::ValidateIdentifier(): a column alias cannot be dot-qualified.
  if (!impl::IsPlainIdentifier(name))
    throw InvalidIdentifier(std::string(name));
  return SelectItem(ToString() + " AS " + std::string(name));
}

std::string Expr::Extract(OperatorPrecedence precedence) const {
  return impl::RenderWithContext(*node_, precedence);
}

std::string Expr::ToString() const { return impl::Render(*node_); }

} // namespace iron_query
