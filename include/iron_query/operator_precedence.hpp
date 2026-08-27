#pragma once

namespace iron_query {

/// @brief Relative binding strength of SQL operators, used by @ref
/// Expr::Extract to decide whether a sub-expression needs to be
/// parenthesized when embedded into a larger expression. From
/// https://www.postgresql.org/docs/current/sql-syntax-lexical.html#SQL-PRECEDENCE
enum class OperatorPrecedence {
  kSymbol,
  kDot,
  kTypecast,
  kIndex,
  kUnaryPlus,
  kCollate,
  kAt,
  kExp,
  kMul,
  kPlus,
  kAnyOther,
  kBetween,
  kCompare,
  kIs,
  kNot,
  kAnd,
  kOr,
  kExtract, // pseudo precedence for "no brackets"
};

} // namespace iron_query
