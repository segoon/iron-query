#include "detail/render.hpp"
#include <iron_query/delete_from.hpp>
#include <stdexcept>
#include <utility>

namespace iron_query {

DeleteFrom::DeleteFrom(const Table &tbl) : from_(tbl.ToStringBracketed()) {}

DeleteFrom DeleteFrom::Where(Condition exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

DeleteFrom DeleteFrom::Returning(SelectItem item) && {
  return std::move(*this).Returning({std::move(item)});
}

DeleteFrom DeleteFrom::Returning(std::initializer_list<SelectItem> items) && {
  returning_ = detail::JoinCsv(detail::RenderAll(items));
  return std::move(*this);
}

std::string DeleteFrom::ToString() const {
  if (from_.empty())
    throw std::logic_error("iron_query: FROM clause is not set");

  auto s = "DELETE FROM " + from_;
  if (!where_.empty())
    s += " WHERE " + where_;
  if (!returning_.empty())
    s += " RETURNING " + returning_;
  return s;
}

} // namespace iron_query
