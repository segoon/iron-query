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

Update Update::Where(Condition exp) && {
  where_ = exp.ToString();
  return std::move(*this);
}

std::string Update::ToString() const {
  if (set_.empty())
    throw std::logic_error("iron_query: SET clause is not set");

  auto s = "UPDATE " + table_ + " SET " + set_;
  if (!where_.empty())
    s += " WHERE " + where_;
  return s;
}

} // namespace iron_query
