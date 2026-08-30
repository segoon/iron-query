#pragma once

/// @file
/// @brief Umbrella header pulling in the whole API. Include a single
/// per-class header instead if you only need part of it.

/// @defgroup statements SQL statements
/// @brief Top-level SQL statement builders: SELECT, INSERT, UPDATE, DELETE,
/// WITH, JOIN, and set operations (UNION/INTERSECT/EXCEPT).

/// @defgroup expressions Expressions and conditions
/// @brief Value-producing (@ref iron_query::Expr) and predicate-producing
/// (@ref iron_query::Condition) expression builders, plus their supporting
/// types (CASE, COLLATE, operator precedence).

/// @defgroup schema Schema and table references
/// @brief Table/column declarations and safe, alias-qualified references to
/// them.

/// @defgroup exceptions Exceptions
/// @brief The exception hierarchy iron_query throws on misuse or invalid
/// input.

/// @defgroup misc Placeholders and integrations
/// @brief Bind-parameter placeholders and optional framework integrations.

#include <iron_query/case_builder.hpp>
#include <iron_query/collation.hpp>
#include <iron_query/column.hpp>
#include <iron_query/condition.hpp>
#include <iron_query/delete_from.hpp>
#include <iron_query/exception.hpp>
#include <iron_query/expr.hpp>
#include <iron_query/insert_into.hpp>
#include <iron_query/join.hpp>
#include <iron_query/operator_precedence.hpp>
#include <iron_query/order_by.hpp>
#include <iron_query/placeholders.hpp>
#include <iron_query/select_expr.hpp>
#include <iron_query/select_item.hpp>
#include <iron_query/set_op.hpp>
#include <iron_query/table.hpp>
#include <iron_query/table_alias.hpp>
#include <iron_query/table_with_columns.hpp>
#include <iron_query/update.hpp>
#include <iron_query/virtual_table.hpp>
#include <iron_query/with.hpp>
