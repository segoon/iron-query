#include "impl/render.hpp"
#include <iron_query/update.hpp>
#include <stdexcept>
#include <utility>

namespace iron_query {

Update::Update(const Table &tbl) : table_(tbl.ToStringBracketed()) {}

Update Update::Set(const Expr &column, const Expr &value) && {
  if (!set_.empty())
    set_ += ", ";
  set_ += column.ToString() + " = " + value.ToString();
  return std::move(*this);
}

Update Update::From(const VirtualTable &tbl) && {
  from_ = tbl.ToStringAsFromItem();
  return std::move(*this);
}

Update Update::Where(Condition exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

Update Update::Returning(SelectItem item) && {
  return std::move(*this).Returning({std::move(item)});
}

Update Update::Returning(std::initializer_list<SelectItem> items) && {
  returning_ = impl::JoinCsv(impl::RenderAll(items));
  return std::move(*this);
}

std::string Update::ToString() const {
  if (set_.empty())
    throw std::logic_error("iron_query: SET clause is not set");

  auto s = "UPDATE " + table_ + " SET " + set_;
  if (!from_.empty())
    s += " FROM " + from_;
  if (!where_.empty())
    s += " WHERE " + where_;
  if (!returning_.empty())
    s += " RETURNING " + returning_;
  return s;
}

} // namespace iron_query
