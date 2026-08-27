#include "render.hpp"

namespace iron_query::impl {

std::string JoinCsv(const std::vector<std::string> &items) {
  std::string s;
  for (const auto &item : items) {
    if (!s.empty())
      s += ", ";
    s += item;
  }
  return s;
}

std::string RenderTerm(const Expr &exp) { return exp.ToString(); }

std::string RenderTerm(const SelectItem &item) { return item.ToString(); }

std::string RenderTerm(const OrderByTerm &term) {
  std::string s = term.expr.ToString();
  if (term.direction == SortDirection::kDescending)
    s += " DESC";
  if (term.nulls_order == NullsOrder::kFirst)
    s += " NULLS FIRST";
  else if (term.nulls_order == NullsOrder::kLast)
    s += " NULLS LAST";
  return s;
}

} // namespace iron_query::impl
