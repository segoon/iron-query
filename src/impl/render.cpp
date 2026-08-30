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

std::string IndentBlock(const std::string &text, int spaces) {
  const std::string prefix(spaces, ' ');
  std::string s = prefix;
  for (char c : text) {
    s += c;
    if (c == '\n')
      s += prefix;
  }
  return s;
}

std::string RenderBinary(const std::string &lhs, const char *op,
                         const std::string &rhs) {
  return lhs + " " + op + " " + rhs;
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
