#include <iron_query/select_item.hpp>

namespace iron_query {

SelectItem::SelectItem(std::string s) : s_(std::move(s)) {}

std::string SelectItem::ToString() const { return s_; }

} // namespace iron_query
