#include "impl/render.hpp"
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

SelectExpr SelectExpr::DistinctOn(Expr exp) && {
  return std::move(*this).DistinctOn({std::move(exp)});
}

SelectExpr SelectExpr::DistinctOn(std::initializer_list<Expr> exps) && {
  distinct_on_ = impl::RenderAll(exps);
  return std::move(*this);
}

SelectExpr SelectExpr::Select(SelectItem item) && {
  return std::move(*this).Select({std::move(item)});
}

SelectExpr SelectExpr::Select(std::initializer_list<SelectItem> items) && {
  select_ = impl::RenderAll(items);
  return std::move(*this);
}

SelectExpr SelectExpr::Where(Condition exp) && {
  if (!where_.empty())
    throw std::logic_error("iron_query: WHERE clause is already set");
  where_ = exp.ToString();
  return std::move(*this);
}

SelectExpr SelectExpr::OrderBy(OrderByTerm term) && {
  return std::move(*this).OrderBy({std::move(term)});
}

SelectExpr SelectExpr::OrderBy(std::initializer_list<OrderByTerm> terms) && {
  order_by_ = impl::RenderAll(terms);
  return std::move(*this);
}

SelectExpr SelectExpr::GroupBy(Expr exp) && {
  return std::move(*this).GroupBy({std::move(exp)});
}

SelectExpr SelectExpr::GroupBy(std::initializer_list<Expr> exps) && {
  group_by_ = impl::RenderAll(exps);
  return std::move(*this);
}

SelectExpr SelectExpr::Having(Condition exp) && {
  if (!having_.empty())
    throw std::logic_error("iron_query: HAVING clause is already set");
  having_ = exp.ToString();
  return std::move(*this);
}

SelectExpr SelectExpr::Limit(int limit) && {
  if (!limit_.empty())
    throw std::logic_error("iron_query: LIMIT clause is already set");
  limit_ = std::to_string(limit);
  return std::move(*this);
}

SelectExpr SelectExpr::Offset(int offset) && {
  if (!offset_.empty())
    throw std::logic_error("iron_query: OFFSET clause is already set");
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
  if (distinct_ && !distinct_on_.empty())
    throw std::logic_error(
        "iron_query: DISTINCT and DISTINCT ON are mutually exclusive");
}

std::string SelectExpr::ToString() const {
  EnsureValid();

  std::string s;
  if (!distinct_on_.empty())
    s = "SELECT DISTINCT ON (" + impl::JoinCsv(distinct_on_) + ") ";
  else
    s = distinct_ ? "SELECT DISTINCT " : "SELECT ";
  s += impl::JoinCsv(select_) + " FROM " + from_;
  if (!where_.empty())
    s += " WHERE " + where_;
  if (!group_by_.empty())
    s += " GROUP BY " + impl::JoinCsv(group_by_);
  if (!having_.empty())
    s += " HAVING " + having_;
  if (!order_by_.empty())
    s += " ORDER BY " + impl::JoinCsv(order_by_);
  if (!limit_.empty())
    s += " LIMIT " + limit_;
  if (!offset_.empty())
    s += " OFFSET " + offset_;
  return s;
}

std::string SelectExpr::ToStringFormatted() const {
  EnsureValid();

  std::string header;
  if (!distinct_on_.empty())
    header = "SELECT DISTINCT ON (" + impl::JoinCsv(distinct_on_) + ")\n";
  else
    header = distinct_ ? "SELECT DISTINCT\n" : "SELECT\n";
  auto s = header + JoinCsvIndented(select_) + "\nFROM\n    " + from_;
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
