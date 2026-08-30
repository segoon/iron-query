#pragma once

#include <stdexcept>
#include <string>

namespace iron_query {

/// @brief The builder was used incorrectly: a clause was set twice, a
/// required clause is missing, or methods were called out of order.
/// Programmer error, not bad input data.
/// @ingroup exceptions
class LogicError : public std::logic_error {
public:
  /// @brief Prepends `"iron_query: "` to `what`.
  explicit LogicError(const std::string &what);
};

/// @brief A value or name given to the builder is invalid. Thrown directly
/// for cases with no more specific subclass below (e.g. "needs at least one
/// argument"); prefer catching a specific subclass when one applies.
/// @ingroup exceptions
class InvalidArgument : public std::invalid_argument {
public:
  /// @brief Prepends `"iron_query: "` to `what`.
  explicit InvalidArgument(const std::string &what);
};

/// @brief `identifier` is not a valid (optionally dot-qualified) SQL
/// identifier, or contains an embedded NUL byte.
/// @ingroup exceptions
class InvalidIdentifier : public InvalidArgument {
public:
  /// @brief Reports `identifier` as invalid.
  explicit InvalidIdentifier(const std::string &identifier);
};

/// @brief `op` is not a syntactically valid SQL operator name.
/// @ingroup exceptions
class InvalidOperator : public InvalidArgument {
public:
  /// @brief Reports `op` as invalid.
  explicit InvalidOperator(const std::string &op);
};

/// @brief A literal value has no valid SQL rendering (NaN/infinity, a
/// string containing a NUL byte, ...).
/// @ingroup exceptions
class InvalidLiteral : public InvalidArgument {
public:
  /// @brief Prepends `"iron_query: "` to `what`.
  explicit InvalidLiteral(const std::string &what);
};

/// @brief `column` was referenced via a `TableWithColumns` alias that
/// doesn't declare it.
/// @ingroup exceptions
class UnknownColumn : public InvalidArgument {
public:
  /// @brief Reports `column` as missing from `table`.
  UnknownColumn(const std::string &column, const std::string &table);
};

} // namespace iron_query
