#include "node.hpp"
#include <type_traits>

namespace iron_query::impl {

namespace {

bool NeedsParens(const Node &child, OperatorPrecedence context) {
  return child.precedence >= context;
}

std::size_t PartSize(const Part &part) {
  return std::visit(
      [](const auto &value) -> std::size_t {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
          return value.size();
        } else {
          return value.node->size +
                 (NeedsParens(*value.node, value.context) ? 2 : 0);
        }
      },
      part);
}

void AppendPart(std::string &out, const Part &part) {
  std::visit(
      [&out](const auto &value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
          out += value;
        } else {
          const bool parens = NeedsParens(*value.node, value.context);
          if (parens)
            out += '(';
          for (const auto &child_part : value.node->parts)
            AppendPart(out, child_part);
          if (parens)
            out += ')';
        }
      },
      part);
}

} // namespace

NodePtr MakeLeaf(std::string text, OperatorPrecedence precedence) {
  const std::size_t size = text.size();
  std::vector<Part> parts;
  parts.emplace_back(std::move(text));
  return std::make_shared<const Node>(Node{precedence, std::move(parts), size});
}

NodePtr MakeNode(OperatorPrecedence precedence, std::vector<Part> parts) {
  std::size_t size = 0;
  for (const auto &part : parts)
    size += PartSize(part);
  return std::make_shared<const Node>(Node{precedence, std::move(parts), size});
}

std::string Render(const Node &node) {
  std::string out;
  out.reserve(node.size);
  for (const auto &part : node.parts)
    AppendPart(out, part);
  return out;
}

std::string RenderWithContext(const Node &node, OperatorPrecedence context) {
  const bool parens = NeedsParens(node, context);
  std::string out;
  out.reserve(node.size + (parens ? 2 : 0));
  if (parens)
    out += '(';
  for (const auto &part : node.parts)
    AppendPart(out, part);
  if (parens)
    out += ')';
  return out;
}

} // namespace iron_query::impl
