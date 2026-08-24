#include <iron_query/table.hpp>
#include <utility>

namespace iron_query {

Table::Table(std::string name) : name_(std::move(name)) {}

Table Table::FromRaw(std::string name) { return Table(std::move(name)); }

std::string Table::ToString() const { return name_; }

std::string Table::ToStringBracketed() const {
  // It is a table with name, no need to bracket it
  return ToString();
}

std::string Table::ToStringAsFromItem() const { return ToString(); }

} // namespace iron_query
