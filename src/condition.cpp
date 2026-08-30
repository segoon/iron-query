#include "impl/node.hpp"
#include <iron_query/condition.hpp>
#include <iron_query/expr.hpp>
#include <utility>

namespace iron_query {

Condition::Condition(std::string s, OperatorPrecedence precedence)
    : node_(impl::MakeLeaf(std::move(s), precedence)) {}

Condition::Condition(std::shared_ptr<const impl::Node> node)
    : node_(std::move(node)) {}

Condition Condition::FromRaw(std::string s) {
  return Condition(std::move(s), OperatorPrecedence::kSymbol);
}

Condition Condition::operator&&(const Condition &other) const {
  return Condition(impl::MakeNode(
      OperatorPrecedence::kAnd,
      {impl::ChildRef{node_, OperatorPrecedence::kAnd}, std::string(" AND "),
       impl::ChildRef{other.node_, OperatorPrecedence::kAnd}}));
}

Condition Condition::operator||(const Condition &other) const {
  return Condition(impl::MakeNode(
      OperatorPrecedence::kOr,
      {impl::ChildRef{node_, OperatorPrecedence::kOr}, std::string(" OR "),
       impl::ChildRef{other.node_, OperatorPrecedence::kOr}}));
}

Condition Condition::operator!() const {
  return Condition(impl::MakeNode(
      OperatorPrecedence::kNot,
      {std::string("NOT "), impl::ChildRef{node_, OperatorPrecedence::kNot}}));
}

std::string Condition::Extract(OperatorPrecedence precedence) const {
  return impl::RenderWithContext(*node_, precedence);
}

std::string Condition::ToString() const { return impl::Render(*node_); }

Condition::operator Expr() && { return Expr(std::move(node_)); }

} // namespace iron_query
