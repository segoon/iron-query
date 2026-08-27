#include <iron_query/order_by.hpp>
#include <utility>

namespace iron_query {

OrderByTerm::OrderByTerm(Expr expr, SortDirection direction)
    : expr(std::move(expr)), direction(direction) {}

} // namespace iron_query
