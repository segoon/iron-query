#include <iron_query/collation.hpp>
#include <utility>

namespace iron_query {

Collation::Collation(std::string name) : name_(std::move(name)) {}

Collation Collation::FromRaw(std::string name) {
  return Collation(std::move(name));
}

std::string Collation::ToString() const { return name_; }

} // namespace iron_query
