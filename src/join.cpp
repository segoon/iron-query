#include "impl/render.hpp"
#include <iron_query/join.hpp>
#include <stdexcept>
#include <string>
#include <utility>

namespace iron_query {

std::string_view Inner::ToString() const { return "INNER"; }
std::string_view Cross::ToString() const { return "CROSS"; }
std::string_view LeftOuter::ToString() const { return "LEFT OUTER"; }
std::string_view RightOuter::ToString() const { return "RIGHT OUTER"; }
std::string_view FullOuter::ToString() const { return "FULL OUTER"; }
std::string_view NaturalInner::ToString() const { return "NATURAL INNER"; }
std::string_view NaturalLeftOuter::ToString() const {
  return "NATURAL LEFT OUTER";
}
std::string_view NaturalRightOuter::ToString() const {
  return "NATURAL RIGHT OUTER";
}
std::string_view NaturalFullOuter::ToString() const {
  return "NATURAL FULL OUTER";
}

// ---------------------------------------------------------------------------
// Join
// ---------------------------------------------------------------------------

Join::Join(const VirtualTable &a, const VirtualTable &b, const JoinKind &kind)
    : a_(a.ToStringBracketed()), b_(b.ToStringBracketed()),
      kind_(kind.ToString()), natural_(kind.IsNatural()) {}

Join Join::On(Condition exp) && {
  if (natural_)
    throw std::logic_error("iron_query: NATURAL JOIN cannot have an ON clause");
  if (!using_.empty())
    throw std::logic_error(
        "iron_query: JOIN cannot have both ON and USING clauses");
  impl::SetOnce(on_, "ON clause", exp.ToString());
  return std::move(*this);
}

Join Join::Using(std::initializer_list<Expr> columns) && {
  if (natural_)
    throw std::logic_error(
        "iron_query: NATURAL JOIN cannot have a USING clause");
  if (!on_.empty())
    throw std::logic_error(
        "iron_query: JOIN cannot have both ON and USING clauses");
  if (columns.size() == 0)
    throw std::invalid_argument(
        "iron_query: USING clause needs at least one column");
  impl::SetOnce(using_, "USING clause",
                impl::JoinCsv(impl::RenderAll(columns)));
  return std::move(*this);
}

std::string Join::ToString() const {
  auto s = a_ + " " + kind_ + " JOIN " + b_;
  if (!on_.empty())
    s += " ON " + on_;
  if (!using_.empty())
    s += " USING (" + using_ + ")";
  return s;
}

std::string Join::ToStringAsFromItem() const { return ToString(); }

} // namespace iron_query
