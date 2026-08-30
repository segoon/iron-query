#include <iron_query/case_builder.hpp>
#include <iron_query/exception.hpp>
#include <utility>

namespace iron_query {

CaseBuilder CaseBuilder::When(Condition cond) && {
  if (has_pending_when_)
    throw LogicError("When() called twice without a matching Then()");
  pending_when_ = cond.ToString();
  has_pending_when_ = true;
  return std::move(*this);
}

CaseBuilder CaseBuilder::Then(Expr result) && {
  if (!has_pending_when_)
    throw LogicError("Then() called without a preceding When()");
  const auto result_str = result.ToString();
  std::string fragment;
  fragment.reserve(5 + pending_when_.size() + 7 + result_str.size() + 1);
  fragment += "WHEN ";
  fragment += pending_when_;
  fragment += " THEN ";
  fragment += result_str;
  fragment += ' ';
  whens_ += fragment;
  has_pending_when_ = false;
  return std::move(*this);
}

CaseBuilder CaseBuilder::Else(Expr result) && {
  else_ = result.ToString();
  return std::move(*this);
}

Expr CaseBuilder::End() const {
  if (whens_.empty())
    throw LogicError("CASE requires at least one When()/Then() pair");

  auto s = "CASE " + whens_;
  if (!else_.empty())
    s += "ELSE " + else_ + " ";
  s += "END";
  return Expr::FromRaw(std::move(s), OperatorPrecedence::kSymbol);
}

CaseBuilder Case() { return CaseBuilder(); }

} // namespace iron_query
