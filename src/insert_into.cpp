#include "detail/render.hpp"
#include <iron_query/insert_into.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace iron_query {

InsertInto::InsertInto(const Table &tbl) : into_(tbl.ToStringBracketed()) {}

namespace {

// Both lists are known only once each setter has run, so the check has to be
// repeated rather than done once at the end.
void EnsureSameArity(const std::vector<std::string> &columns,
                     const std::vector<std::vector<std::string>> &rows) {
  if (columns.empty() || rows.empty())
    return;
  for (const auto &row : rows) {
    if (row.size() != columns.size())
      throw std::logic_error("iron_query: " + std::to_string(columns.size()) +
                             " columns but a row with " +
                             std::to_string(row.size()) + " values");
  }
}

} // namespace

InsertInto InsertInto::Columns(std::initializer_list<Expr> cols) && {
  columns_ = detail::RenderAll(cols);
  EnsureSameArity(columns_, rows_);
  return std::move(*this);
}

InsertInto InsertInto::Values(std::initializer_list<Expr> vals) && {
  rows_ = {detail::RenderAll(vals)};
  EnsureSameArity(columns_, rows_);
  return std::move(*this);
}

InsertInto
InsertInto::Rows(std::initializer_list<std::initializer_list<Expr>> rows) && {
  rows_.clear();
  rows_.reserve(rows.size());
  for (const auto &row : rows)
    rows_.push_back(detail::RenderAll(row));
  EnsureSameArity(columns_, rows_);
  return std::move(*this);
}

InsertInto InsertInto::Returning(SelectItem item) && {
  return std::move(*this).Returning({std::move(item)});
}

InsertInto InsertInto::Returning(std::initializer_list<SelectItem> items) && {
  returning_ = detail::JoinCsv(detail::RenderAll(items));
  return std::move(*this);
}

InsertInto InsertInto::OnConflictDoNothing() && {
  on_conflict_ = "ON CONFLICT DO NOTHING";
  return std::move(*this);
}

InsertInto
InsertInto::OnConflictDoNothing(std::initializer_list<Expr> target_cols) && {
  on_conflict_ = "ON CONFLICT (" +
                 detail::JoinCsv(detail::RenderAll(target_cols)) +
                 ") DO NOTHING";
  return std::move(*this);
}

InsertInto InsertInto::OnConflictDoUpdate(
    std::initializer_list<Expr> target_cols,
    std::initializer_list<std::pair<Expr, Expr>> assignments) && {
  if (assignments.size() == 0)
    throw std::invalid_argument(
        "iron_query: ON CONFLICT DO UPDATE needs at least one assignment");

  std::string set;
  for (const auto &[column, value] : assignments) {
    if (!set.empty())
      set += ", ";
    set += column.ToString() + " = " + value.ToString();
  }
  on_conflict_ = "ON CONFLICT (" +
                 detail::JoinCsv(detail::RenderAll(target_cols)) +
                 ") DO UPDATE SET " + set;
  return std::move(*this);
}

std::string InsertInto::ToString() const {
  if (columns_.empty())
    throw std::logic_error("iron_query: no columns to insert into");
  if (rows_.empty())
    throw std::logic_error("iron_query: no values to insert");
  EnsureSameArity(columns_, rows_);

  std::string s =
      "INSERT INTO " + into_ + " (" + detail::JoinCsv(columns_) + ") VALUES ";
  for (std::size_t i = 0; i < rows_.size(); ++i) {
    if (i != 0)
      s += ", ";
    s += "(" + detail::JoinCsv(rows_[i]) + ")";
  }
  if (!on_conflict_.empty())
    s += " " + on_conflict_;
  if (!returning_.empty())
    s += " RETURNING " + returning_;
  return s;
}

} // namespace iron_query
