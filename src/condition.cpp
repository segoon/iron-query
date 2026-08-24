#include <iron_query/condition.hpp>
#include <iron_query/expr.hpp>

namespace iron_query {

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

} // namespace iron_query
