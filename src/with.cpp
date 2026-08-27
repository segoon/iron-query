#include "impl/identifier.hpp"
#include <iron_query/with.hpp>
#include <utility>

namespace iron_query {

WithQuery::WithQuery(std::string ctes, std::string main)
    : ctes_(std::move(ctes)), main_(std::move(main)) {}

std::string WithQuery::ToString() const {
  return "WITH " + ctes_ + " " + main_;
}

WithBuilder::WithBuilder(std::string name, const VirtualTable &query)
    : ctes_(std::move(name) + " AS " + query.ToStringBracketed()) {}

WithBuilder WithBuilder::With(std::string name, const VirtualTable &query) && {
  impl::ValidateIdentifier(name);
  ctes_ += ", " + std::move(name) + " AS " + query.ToStringBracketed();
  return std::move(*this);
}

WithQuery WithBuilder::Main(const VirtualTable &query) && {
  return WithQuery(std::move(ctes_), query.ToString());
}

WithBuilder With(std::string name, const VirtualTable &query) {
  impl::ValidateIdentifier(name);
  return WithBuilder(std::move(name), query);
}

} // namespace iron_query
