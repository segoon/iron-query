#include <iron_query/delete_from.hpp>
#include <stdexcept>
#include <utility>

namespace iron_query {

DeleteFrom::DeleteFrom(const Table &tbl) : from_(tbl.ToStringBracketed()) {}

DeleteFrom DeleteFrom::Where(Condition exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

std::string DeleteFrom::ToString() const {
  if (from_.empty())
    throw std::logic_error("iron_query: FROM clause is not set");

  auto s = "DELETE FROM " + from_;
  if (!where_.empty())
    s += " WHERE " + where_;
  return s;
}

} // namespace iron_query
