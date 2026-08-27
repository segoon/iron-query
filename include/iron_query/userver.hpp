#pragma once

/// @file
/// @brief Optional integration with userver's `storages::Query`. Not pulled
/// in by @ref iron_query.hpp: iron-query itself does not depend on userver,
/// so include this header directly in projects that have userver on their
/// include path.

#include <optional>
#include <utility>

#include <userver/storages/query.hpp>

namespace iron_query::userver {

/// @brief Renders a finalized IronQuery builder (@ref SelectExpr, @ref
/// InsertInto, @ref Update, @ref DeleteFrom, @ref SetOp, @ref WithQuery, or
/// anything else exposing `std::string ToString() const`) into a userver
/// `storages::Query`, ready to pass to a driver's `Execute`/`Select`.
///
/// @note Values for `_1`..`_10` placeholders are still the caller's
/// responsibility, exactly as with a bare `ToString()` call.
template <typename Builder>
::storages::Query ToQuery(
    const Builder &builder,
    std::optional<::storages::Query::Name> name = std::nullopt,
    ::storages::Query::LogMode log_mode = ::storages::Query::LogMode::kFull) {
  return ::storages::Query(builder.ToString(), std::move(name), log_mode);
}

} // namespace iron_query::userver
