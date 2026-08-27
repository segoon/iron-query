#include <iron_query/set_op.hpp>

namespace iron_query {

std::string_view Union::ToString() const { return "UNION"; }
std::string_view UnionAll::ToString() const { return "UNION ALL"; }
std::string_view Intersect::ToString() const { return "INTERSECT"; }
std::string_view Except::ToString() const { return "EXCEPT"; }

// ---------------------------------------------------------------------------
// SetOp
// ---------------------------------------------------------------------------

SetOp::SetOp(const VirtualTable &a, const VirtualTable &b,
             const SetOpKind &kind)
    : a_(a.ToStringBracketed()), b_(b.ToStringBracketed()),
      kind_(kind.ToString()) {}

std::string SetOp::ToString() const { return a_ + " " + kind_ + " " + b_; }

} // namespace iron_query
