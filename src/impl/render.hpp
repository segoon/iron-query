#pragma once

#include <initializer_list>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <iron_query/exception.hpp>
#include <iron_query/expr.hpp>
#include <iron_query/order_by.hpp>
#include <iron_query/select_item.hpp>

namespace iron_query::impl {

/// @brief Joins already-rendered terms with ", ".
std::string JoinCsv(const std::vector<std::string> &items);

/// @brief Indents every line of `text` by `spaces` spaces, for splicing a
/// pre-rendered multi-line block under a clause header.
std::string IndentBlock(const std::string &text, int spaces);

/// @brief Joins two already-`Extract`ed operands around an infix operator:
/// `lhs + " " + op + " " + rhs`. Shared by `Expr::BinaryOp` and the handful
/// of built-in infix operators (`Condition`'s `&&`/`||`, the free comparison
/// operators) that render the same shape but return `Condition`, not `Expr`.
std::string RenderBinary(const std::string &lhs, const char *op,
                         const std::string &rhs);

/// @brief Whether a clause's storage holds no value yet: `.empty()` for a
/// `std::string`/`std::vector`, `.has_value()` for a `std::optional`.
template <typename T> bool IsEmpty(const T &field) { return field.empty(); }
template <typename T> bool IsEmpty(const std::optional<T> &field) {
  return !field.has_value();
}

/// @brief Throws if `field` (a clause's storage) was already set, so a
/// builder method can reject a second call instead of silently discarding
/// the first one.
template <typename T> void EnsureNotSet(const T &field, const char *what) {
  if (!IsEmpty(field))
    throw LogicError(std::string(what) + " is already set");
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

/// @brief `RenderAll`, for a clause stored as a live `std::vector<T>` rather
/// than an `std::initializer_list<T>` (used by clauses that defer rendering
/// until the final `ToString()`/`ToStringFormatted()` call).
template <typename T>
std::vector<std::string> RenderAll(const std::vector<T> &terms) {
  std::vector<std::string> rendered;
  rendered.reserve(terms.size());
  for (const auto &term : terms)
    rendered.push_back(RenderTerm(term));
  return rendered;
}

} // namespace iron_query::impl
