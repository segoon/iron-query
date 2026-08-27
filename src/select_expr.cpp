#include "detail/render.hpp"
#include <iron_query/select_expr.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace iron_query {

SelectExpr::SelectExpr(const VirtualTable &tbl)
    : from_(tbl.ToStringAsFromItem()) {}

SelectExpr SelectExpr::Distinct() && {
  distinct_ = true;
  return std::move(*this);
}

SelectExpr SelectExpr::Select(SelectItem item) && {
  return std::move(*this).Select({std::move(item)});
}

SelectExpr SelectExpr::Select(std::initializer_list<SelectItem> items) && {
  select_ = detail::RenderAll(items);
  return std::move(*this);
}

SelectExpr SelectExpr::Where(Condition exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

SelectExpr SelectExpr::OrderBy(OrderByTerm term) && {
  return std::move(*this).OrderBy({std::move(term)});
}

SelectExpr SelectExpr::OrderBy(std::initializer_list<OrderByTerm> terms) && {
  order_by_ = detail::RenderAll(terms);
  return std::move(*this);
}

SelectExpr SelectExpr::GroupBy(Expr exp) && {
  return std::move(*this).GroupBy({std::move(exp)});
}

SelectExpr SelectExpr::GroupBy(std::initializer_list<Expr> exps) && {
  group_by_ = detail::RenderAll(exps);
  return std::move(*this);
}

SelectExpr SelectExpr::Having(Condition exp) && {
  having_ = exp.ToString();
  return std::move(*this);
}

SelectExpr SelectExpr::Limit(int limit) && {
  limit_ = std::to_string(limit);
  return std::move(*this);
}

SelectExpr SelectExpr::Offset(int offset) && {
  offset_ = std::to_string(offset);
  return std::move(*this);
}

namespace {

std::string JoinCsvIndented(const std::vector<std::string> &items) {
  std::string s;
  for (const auto &item : items) {
    if (!s.empty())
      s += ",\n";
    s += "    " + item;
  }
  return s;
}

} // namespace

void SelectExpr::EnsureValid() const {
  if (select_.empty())
    throw std::logic_error("iron_query: SELECT clause is not set");
  if (from_.empty())
    throw std::logic_error("iron_query: FROM clause is not set");
}

std::string SelectExpr::ToString() const {
  EnsureValid();

  std::string s = distinct_ ? "SELECT DISTINCT " : "SELECT ";
  s += detail::JoinCsv(select_) + " FROM " + from_;
  if (!where_.empty())
    s += " WHERE " + where_;
  if (!group_by_.empty())
    s += " GROUP BY " + detail::JoinCsv(group_by_);
  if (!having_.empty())
    s += " HAVING " + having_;
  if (!order_by_.empty())
    s += " ORDER BY " + detail::JoinCsv(order_by_);
  if (!limit_.empty())
    s += " LIMIT " + limit_;
  if (!offset_.empty())
    s += " OFFSET " + offset_;
  return s;
}

std::string SelectExpr::ToStringFormatted() const {
  EnsureValid();

  auto s = std::string(distinct_ ? "SELECT DISTINCT\n" : "SELECT\n") +
           JoinCsvIndented(select_) + "\nFROM\n    " + from_;
  if (!where_.empty())
    s += "\nWHERE\n    " + where_;
  if (!group_by_.empty())
    s += "\nGROUP BY\n" + JoinCsvIndented(group_by_);
  if (!having_.empty())
    s += "\nHAVING\n    " + having_;
  if (!order_by_.empty())
    s += "\nORDER BY\n" + JoinCsvIndented(order_by_);
  if (!limit_.empty())
    s += "\nLIMIT\n    " + limit_;
  if (!offset_.empty())
    s += "\nOFFSET\n    " + offset_;
  return s;
}

SelectExpr From(const VirtualTable &tbl) { return SelectExpr(tbl); }

} // namespace iron_query
