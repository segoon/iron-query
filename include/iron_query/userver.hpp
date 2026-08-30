#pragma once

/// @file
/// @brief Optional integration with userver's `storages::Query`. Not pulled
/// in by @ref iron_query.hpp — iron-query itself does not depend on userver,
/// so include this header directly in projects that have userver on their
/// include path.

#include <utility>

#include <userver/storages/query.hpp>

namespace iron_query::userver {

/// @brief Renders a finalized IronQuery builder (@ref iron_query::SelectExpr,
/// @ref iron_query::InsertInto, @ref iron_query::Update, @ref
/// iron_query::DeleteFrom, @ref iron_query::SetOp, @ref
/// iron_query::WithQuery, or anything else exposing `std::string ToString()
/// const`) into a userver `storages::Query`, ready to pass to a driver's
/// `Execute`/`Select`.
///
/// @note Values for `_1`..`_10` placeholders are still the caller's
/// responsibility, exactly as with a bare `ToString()` call.
/// @ingroup misc
template <typename Builder>
::storages::Query ToQuery(
    const Builder &builder, ::storages::Query::Name name,
    ::storages::Query::LogMode log_mode = ::storages::Query::LogMode::kFull) {
  return ::storages::Query(builder.ToString(), std::move(name), log_mode);
}

} // namespace iron_query::userver
