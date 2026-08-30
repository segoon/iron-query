#include <iron_query/exception.hpp>

namespace iron_query {

LogicError::LogicError(const std::string &what)
    : std::logic_error("iron_query: " + what) {}

InvalidArgument::InvalidArgument(const std::string &what)
    : std::invalid_argument("iron_query: " + what) {}

InvalidIdentifier::InvalidIdentifier(const std::string &identifier)
    : InvalidArgument("invalid identifier: " + identifier) {}

InvalidOperator::InvalidOperator(const std::string &op)
    : InvalidArgument("invalid operator name: " + op) {}

InvalidLiteral::InvalidLiteral(const std::string &what)
    : InvalidArgument(what) {}

UnknownColumn::UnknownColumn(const std::string &column,
                             const std::string &table)
    : InvalidArgument("'" + column + "' is not a column of table '" + table +
                      "'") {}

} // namespace iron_query
