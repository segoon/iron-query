#include "render.hpp"
#include <algorithm>
#include <cstring>

namespace iron_query::impl {

std::string JoinCsv(const std::vector<std::string> &items) {
  std::string s;
  if (items.empty())
    return s;

  std::size_t size = 2 * (items.size() - 1);
  for (const auto &item : items)
    size += item.size();
  s.reserve(size);

  for (const auto &item : items) {
    if (!s.empty())
      s += ", ";
    s += item;
  }
  return s;
}

std::string IndentBlock(const std::string &text, int spaces) {
  const std::string prefix(spaces, ' ');
  const auto newlines = std::count(text.begin(), text.end(), '\n');
  std::string s;
  s.reserve(prefix.size() + text.size() +
            static_cast<std::size_t>(newlines) * prefix.size());
  s += prefix;
  for (char c : text) {
    s += c;
    if (c == '\n')
      s += prefix;
  }
  return s;
}

std::string RenderBinary(const std::string &lhs, const char *op,
                         const std::string &rhs) {
  const auto op_len = std::strlen(op);
  std::string s;
  s.reserve(lhs.size() + rhs.size() + op_len + 2);
  s += lhs;
  s += ' ';
  s.append(op, op_len);
  s += ' ';
  s += rhs;
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
