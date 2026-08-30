#pragma once

#include <cstddef>
#include <iron_query/operator_precedence.hpp>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace iron_query::impl {

struct Node;

/// @brief Shared, immutable handle to a rendered-lazily expression node.
using NodePtr = std::shared_ptr<const Node>;

/// @brief A child expression embedded at a given precedence context: it is
/// wrapped in parentheses when rendered iff its own top-level precedence
/// does not bind at least as tightly as `context` (the same rule
/// `Expr::Extract`/`Condition::Extract` document).
struct ChildRef {
  NodePtr node;
  OperatorPrecedence context;
};

/// @brief One piece of a node's rendering: either a literal fragment (an
/// operator, keyword, or punctuation) or a reference to a child node.
using Part = std::variant<std::string, ChildRef>;

/// @brief A lazily-rendered expression/condition tree node. Immutable once
/// built; `size` is the exact rendered length (including any parentheses
/// each child part contributes), computed once at construction so the final
/// render can `reserve()` exactly instead of growing incrementally.
struct Node {
  OperatorPrecedence precedence;
  std::vector<Part> parts;
  std::size_t size;
};

/// @brief Builds a leaf node wrapping trusted, already-rendered text.
NodePtr MakeLeaf(std::string text, OperatorPrecedence precedence);

/// @brief Builds a composite node from its parts, computing `size` from the
/// (already known) size of each part.
NodePtr MakeNode(OperatorPrecedence precedence, std::vector<Part> parts);

/// @brief Renders the whole tree, unparenthesized at the top level.
std::string Render(const Node &node);

/// @brief Renders the tree, adding a top-level parenthesis pair iff `node`'s
/// precedence does not bind at least as tightly as `context`.
std::string RenderWithContext(const Node &node, OperatorPrecedence context);

} // namespace iron_query::impl
