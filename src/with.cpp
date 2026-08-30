#include "impl/identifier.hpp"
#include "impl/render.hpp"
#include <algorithm>
#include <iron_query/exception.hpp>
#include <iron_query/with.hpp>
#include <utility>

namespace iron_query {

WithQuery::WithQuery(std::string ctes, std::string main,
                     std::string ctes_formatted, std::string main_formatted)
    : ctes_(std::move(ctes)), main_(std::move(main)),
      ctes_formatted_(std::move(ctes_formatted)),
      main_formatted_(std::move(main_formatted)) {}

std::string WithQuery::ToString() const {
  return "WITH " + ctes_ + " " + main_;
}

std::string WithQuery::ToStringFormatted() const {
  return "WITH\n" + impl::IndentBlock(ctes_formatted_, 4) + "\n" +
         main_formatted_;
}

WithBuilder::WithBuilder(std::string name, const VirtualTable &query)
    : ctes_(name + " AS " + query.ToStringBracketed()),
      ctes_formatted_(name + " AS (\n" +
                      impl::IndentBlock(query.ToStringFormatted(), 4) + "\n)"),
      cte_names_{std::move(name)} {}

WithBuilder WithBuilder::With(std::string name, const VirtualTable &query) && {
  impl::ValidateIdentifier(name);
  if (std::find(cte_names_.begin(), cte_names_.end(), name) != cte_names_.end())
    throw LogicError("duplicate CTE name: " + name);
  ctes_ += ", " + name + " AS " + query.ToStringBracketed();
  ctes_formatted_ += ",\n" + name + " AS (\n" +
                     impl::IndentBlock(query.ToStringFormatted(), 4) + "\n)";
  cte_names_.push_back(std::move(name));
  return std::move(*this);
}

WithQuery WithBuilder::Main(const VirtualTable &query) && {
  return WithQuery(std::move(ctes_), query.ToString(),
                   std::move(ctes_formatted_), query.ToStringFormatted());
}

WithBuilder With(std::string name, const VirtualTable &query) {
  impl::ValidateIdentifier(name);
  return WithBuilder(std::move(name), query);
}

} // namespace iron_query
