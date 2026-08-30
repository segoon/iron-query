#pragma once

#include <gtest/gtest.h>
#include <iron_query/iron_query.hpp>

using namespace iron_query;

namespace {
// EXPECT_THROW discards the value of the statement it evaluates, which is
// the whole point of the macro, but that trips -Wunused-result against our
// [[nodiscard]] builder types. Wrapping the statement in Ignore() marks the
// discard as intentional at each call site.
template <typename T>
void Ignore(T &&) {} // NOLINT(readability-named-parameter)
} // namespace
