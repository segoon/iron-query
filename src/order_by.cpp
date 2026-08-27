#include <iron_query/order_by.hpp>
#include <utility>

namespace iron_query {

OrderByTerm::OrderByTerm(Expr expr, SortDirection direction,
                         NullsOrder nulls_order)
    : expr(std::move(expr)), direction(direction), nulls_order(nulls_order) {}

} // namespace iron_query
