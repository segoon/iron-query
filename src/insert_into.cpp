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
                     const std::vector<std::string> &values) {
  if (columns.empty() || values.empty())
    return;
  if (columns.size() != values.size())
    throw std::logic_error("iron_query: " + std::to_string(columns.size()) +
                           " columns but " + std::to_string(values.size()) +
                           " values");
}

} // namespace

InsertInto InsertInto::Columns(std::initializer_list<Expr> cols) && {
  columns_ = detail::RenderAll(cols);
  EnsureSameArity(columns_, values_);
  return std::move(*this);
}

InsertInto InsertInto::Values(std::initializer_list<Expr> vals) && {
  values_ = detail::RenderAll(vals);
  EnsureSameArity(columns_, values_);
  return std::move(*this);
}

std::string InsertInto::ToString() const {
  if (columns_.empty())
    throw std::logic_error("iron_query: no columns to insert into");
  if (values_.empty())
    throw std::logic_error("iron_query: no values to insert");
  EnsureSameArity(columns_, values_);

  return "INSERT INTO " + into_ + " (" + detail::JoinCsv(columns_) +
         ") VALUES (" + detail::JoinCsv(values_) + ")";
}

} // namespace iron_query
