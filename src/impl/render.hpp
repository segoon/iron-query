#pragma once

#include <initializer_list>
#include <string>
#include <vector>

#include <iron_query/expr.hpp>
#include <iron_query/order_by.hpp>
#include <iron_query/select_item.hpp>

namespace iron_query::impl {

/// @brief Joins already-rendered terms with ", ".
std::string JoinCsv(const std::vector<std::string> &items);

/// @brief Renders one comma-separated clause term.
std::string RenderTerm(const Expr &exp);
/// @brief Renders one comma-separated clause term.
std::string RenderTerm(const SelectItem &item);
/// @brief Renders one comma-separated clause term.
std::string RenderTerm(const OrderByTerm &term);

// Gives every comma-separated clause a single assignment site, so its
// one-term and many-term setters cannot disagree on replace-vs-append.
template <typename T>
std::vector<std::string> RenderAll(std::initializer_list<T> terms) {
  std::vector<std::string> rendered;
  rendered.reserve(terms.size());
  for (const auto &term : terms)
    rendered.push_back(RenderTerm(term));
  return rendered;
}

} // namespace iron_query::impl
