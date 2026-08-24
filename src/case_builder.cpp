#include <iron_query/case_builder.hpp>
#include <stdexcept>
#include <utility>

namespace iron_query {

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

} // namespace iron_query
