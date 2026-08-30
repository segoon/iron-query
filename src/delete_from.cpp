#include "impl/render.hpp"
#include <iron_query/delete_from.hpp>
#include <iron_query/exception.hpp>
#include <utility>

namespace iron_query {

DeleteFrom::DeleteFrom(const Table &tbl) : from_(tbl.ToStringBracketed()) {}

DeleteFrom DeleteFrom::Using(const VirtualTable &tbl) && {
  impl::SetOnce(using_, "USING clause", tbl.ToStringAsFromItem());
  return std::move(*this);
}

DeleteFrom DeleteFrom::Where(Condition exp) && {
  impl::SetOnce(where_, "WHERE clause", exp.ToString());
  return std::move(*this);
}

DeleteFrom DeleteFrom::Returning(SelectItem item) && {
  return std::move(*this).Returning({std::move(item)});
}

DeleteFrom DeleteFrom::Returning(std::initializer_list<SelectItem> items) && {
  impl::SetOnce(returning_, "RETURNING clause",
                impl::JoinCsv(impl::RenderAll(items)));
  return std::move(*this);
}

std::string DeleteFrom::ToString() const {
  if (from_.empty())
    throw LogicError("FROM clause is not set");

  auto s = "DELETE FROM " + from_;
  if (!using_.empty())
    s += " USING " + using_;
  if (!where_.empty())
    s += " WHERE " + where_;
  if (!returning_.empty())
    s += " RETURNING " + returning_;
  return s;
}

} // namespace iron_query
