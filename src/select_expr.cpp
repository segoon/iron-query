#include "impl/render.hpp"
#include <cassert>
#include <iron_query/exception.hpp>
#include <iron_query/select_expr.hpp>
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
  impl::SetOnce(distinct_on_, "DISTINCT ON clause", std::vector<Expr>(exps));
  return std::move(*this);
}

SelectExpr SelectExpr::Select(SelectItem item) && {
  return std::move(*this).Select({std::move(item)});
}

SelectExpr SelectExpr::Select(std::initializer_list<SelectItem> items) && {
  impl::SetOnce(select_, "SELECT clause", std::vector<SelectItem>(items));
  return std::move(*this);
}

SelectExpr SelectExpr::Where(Condition exp) && {
  impl::SetOnce(where_, "WHERE clause", std::move(exp));
  return std::move(*this);
}

SelectExpr SelectExpr::OrderBy(OrderByTerm term) && {
  return std::move(*this).OrderBy({std::move(term)});
}

SelectExpr SelectExpr::OrderBy(std::initializer_list<OrderByTerm> terms) && {
  impl::SetOnce(order_by_, "ORDER BY clause", std::vector<OrderByTerm>(terms));
  return std::move(*this);
}

SelectExpr SelectExpr::GroupBy(Expr exp) && {
  return std::move(*this).GroupBy({std::move(exp)});
}

SelectExpr SelectExpr::GroupBy(std::initializer_list<Expr> exps) && {
  impl::SetOnce(group_by_, "GROUP BY clause", std::vector<Expr>(exps));
  return std::move(*this);
}

SelectExpr SelectExpr::Having(Condition exp) && {
  impl::SetOnce(having_, "HAVING clause", std::move(exp));
  return std::move(*this);
}

SelectExpr SelectExpr::Limit(int limit) && {
  impl::SetOnce(limit_, "LIMIT clause", limit);
  return std::move(*this);
}

SelectExpr SelectExpr::Offset(int offset) && {
  impl::SetOnce(offset_, "OFFSET clause", offset);
  return std::move(*this);
}

namespace {

std::string JoinCsvIndented(const std::vector<std::string> &items) {
  std::string s;
  if (items.empty())
    return s;

  std::size_t size = 6 * (items.size() - 1) + 4;
  for (const auto &item : items)
    size += item.size();
  s.reserve(size);

  for (const auto &item : items) {
    if (!s.empty())
      s += ",\n";
    s += "    ";
    s += item;
  }
  return s;
}

} // namespace

void SelectExpr::EnsureValid() const {
  if (!select_)
    throw LogicError("SELECT clause is not set");
  if (from_.empty())
    throw LogicError("FROM clause is not set");
  if (distinct_ && distinct_on_)
    throw LogicError("DISTINCT and DISTINCT ON are mutually exclusive");
}

std::string SelectExpr::ToString() const {
  EnsureValid();
  assert(select_.has_value());

  std::string s;
  if (distinct_on_)
    s = "SELECT DISTINCT ON (" + impl::JoinCsv(impl::RenderAll(*distinct_on_)) +
        ") ";
  else
    s = distinct_ ? "SELECT DISTINCT " : "SELECT ";
  s += impl::JoinCsv(impl::RenderAll(*select_)) + " FROM " + from_;
  if (where_)
    s += " WHERE " + where_->ToString();
  if (group_by_)
    s += " GROUP BY " + impl::JoinCsv(impl::RenderAll(*group_by_));
  if (having_)
    s += " HAVING " + having_->ToString();
  if (order_by_)
    s += " ORDER BY " + impl::JoinCsv(impl::RenderAll(*order_by_));
  if (limit_)
    s += " LIMIT " + std::to_string(*limit_);
  if (offset_)
    s += " OFFSET " + std::to_string(*offset_);
  return s;
}

std::string SelectExpr::ToStringFormatted() const {
  EnsureValid();
  assert(select_.has_value());

  std::string header;
  if (distinct_on_)
    header = "SELECT DISTINCT ON (" +
             impl::JoinCsv(impl::RenderAll(*distinct_on_)) + ")\n";
  else
    header = distinct_ ? "SELECT DISTINCT\n" : "SELECT\n";
  auto s = header + JoinCsvIndented(impl::RenderAll(*select_)) +
           "\nFROM\n    " + from_;
  if (where_)
    s += "\nWHERE\n    " + where_->ToString();
  if (group_by_)
    s += "\nGROUP BY\n" + JoinCsvIndented(impl::RenderAll(*group_by_));
  if (having_)
    s += "\nHAVING\n    " + having_->ToString();
  if (order_by_)
    s += "\nORDER BY\n" + JoinCsvIndented(impl::RenderAll(*order_by_));
  if (limit_)
    s += "\nLIMIT\n    " + std::to_string(*limit_);
  if (offset_)
    s += "\nOFFSET\n    " + std::to_string(*offset_);
  return s;
}

SelectExpr From(const VirtualTable &tbl) { return SelectExpr(tbl); }

} // namespace iron_query
