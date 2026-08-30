#pragma once

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <iron_query/expr.hpp>
#include <iron_query/order_by.hpp>
#include <iron_query/select_item.hpp>

namespace iron_query::impl {

/// @brief Joins already-rendered terms with ", ".
std::string JoinCsv(const std::vector<std::string> &items);

/// @brief Indents every line of `text` by `spaces` spaces, for splicing a
/// pre-rendered multi-line block under a clause header.
std::string IndentBlock(const std::string &text, int spaces);

/// @brief Throws if `field` (a clause's storage, either a `std::string` or a
/// `std::vector<std::string>`) was already set, so a builder method can
/// reject a second call instead of silently discarding the first one.
template <typename T> void EnsureNotSet(const T &field, const char *what) {
  if (!field.empty())
    throw std::logic_error(std::string("iron_query: ") + what +
                           " is already set");
}

/// @brief `EnsureNotSet(field, what)` followed by `field = std::move(value)`,
/// for the common case where the whole clause is one direct assignment.
template <typename T, typename U>
void SetOnce(T &field, const char *what, U value) {
  EnsureNotSet(field, what);
  field = std::move(value);
}

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
