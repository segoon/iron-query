#include "test_helpers.hpp"

#include <cstdint>
#include <limits>

TEST(Expr, symbol) {
  EXPECT_EQ(Expr::FromRaw("x").ToString(), "x");
  EXPECT_EQ(Expr::FromRaw(std::string("x")).ToString(), "x");
  EXPECT_EQ(Expr(1).ToString(), "1");
}

TEST(Expr, Logical) {
  EXPECT_EQ(((Expr(1) < 2) && (Expr(2) == Expr::FromRaw("age"))).ToString(),
            "1 < 2 AND 2 = age");
  EXPECT_EQ(((Condition::FromRaw("x") && Condition::FromRaw("y")) ||
             Condition::FromRaw("z"))
                .ToString(),
            "x AND y OR z");
  EXPECT_EQ(((Condition::FromRaw("x") || Condition::FromRaw("y")) &&
             Condition::FromRaw("z"))
                .ToString(),
            "(x OR y) AND z");
}

TEST(Expr, LogicalMath) {
  EXPECT_EQ((Expr(1) < 2 && Expr(3) * 2 + 3 == 4).ToString(),
            "1 < 2 AND 3 * 2 + 3 = 4");
  EXPECT_EQ((Expr(1) + 2 + 3).ToString(), "(1 + 2) + 3");
  EXPECT_EQ(((Expr(1) + 2) * 3).ToString(), "(1 + 2) * 3");
}

TEST(Expr, Not) {
  EXPECT_EQ((!(Expr(1) < 2)).ToString(), "NOT 1 < 2");
  EXPECT_EQ((!!(Expr(1) < 2)).ToString(), "NOT (NOT 1 < 2)");
}

TEST(Expr, UnaryMinus) {
  EXPECT_EQ((-Expr::FromRaw("a")).ToString(), "-a");
  EXPECT_EQ((-(Expr::FromRaw("a") + 1)).ToString(), "-(a + 1)");
}

TEST(Expr, UnaryMinusDouble) {
  EXPECT_EQ((-(-Expr::FromRaw("a"))).ToString(), "-(-a)");
}

TEST(Expr, UnaryNot) {
  EXPECT_EQ((!Expr::FromRaw("is_admin")).ToString(), "NOT is_admin");
}

TEST(Expr, Concat) {
  EXPECT_EQ(Expr::FromRaw("a").Concat(Expr::FromRaw("b")).ToString(), "a || b");
  EXPECT_EQ((Expr::FromRaw("a") + 1).Concat(Expr::FromRaw("b")).ToString(),
            "a + 1 || b");
  EXPECT_EQ(Expr::FromRaw("a")
                .Concat(Expr::FromRaw("b"))
                .Dot(Expr::FromRaw("c"))
                .ToString(),
            "(a || b).c");
}

TEST(Expr, Dot) {
  EXPECT_EQ(Expr::FromRaw("foo").Dot(Expr::FromRaw("bar")).ToString(),
            "foo.bar");
}

TEST(Expr, Cast) {
  EXPECT_EQ(Expr::FromRaw("1").CastRaw("TEXT").ToString(), "CAST (1 AS TEXT)");
}

TEST(Expr, Collate) {
  EXPECT_EQ(Expr::FromRaw("a").Collate(Collation::FromRaw(R"("C")")).ToString(),
            R"(a COLLATE "C")");
}

TEST(Expr, Index) {
  EXPECT_EQ(Expr::FromRaw("a")[Expr::FromRaw("b")].ToString(), "a[b]");
}

TEST(Expr, IndexPrecedence) {
  // Subscripting binds looser than ".", so the subscript needs bracketing
  // once it is embedded into a tighter context.
  EXPECT_EQ(
      Expr::FromRaw("a")[Expr::FromRaw("b")].Dot(Expr::FromRaw("c")).ToString(),
      "(a[b]).c");
  EXPECT_EQ((Expr::FromRaw("a")[Expr::FromRaw("b")] + 1).ToString(),
            "a[b] + 1");
}

TEST(Expr, Exp) { EXPECT_EQ((Expr::FromRaw("a") ^ 2).ToString(), "a ^ 2"); }

TEST(Expr, Between) {
  EXPECT_EQ(Expr::FromRaw("a").Between(1, 2).ToString(), "a BETWEEN 1 AND 2");
}

TEST(Expr, NotBetween) {
  EXPECT_EQ(Expr::FromRaw("a").NotBetween(1, 2).ToString(),
            "a NOT BETWEEN 1 AND 2");
}

TEST(Expr, Like) {
  EXPECT_EQ(Expr::FromRaw("a").Like(Expr::FromRaw("a%b")).ToString(),
            "a LIKE a%b");
}

TEST(Expr, NotLike) {
  EXPECT_EQ(Expr::FromRaw("a").NotLike(Expr::FromRaw("a%b")).ToString(),
            "a NOT LIKE a%b");
}

TEST(Expr, ILike) {
  EXPECT_EQ(Expr::FromRaw("a").ILike(Expr::FromRaw("a%b")).ToString(),
            "a ILIKE a%b");
}

TEST(Expr, NotILike) {
  EXPECT_EQ(Expr::FromRaw("a").NotILike(Expr::FromRaw("a%b")).ToString(),
            "a NOT ILIKE a%b");
}

TEST(Expr, SimilarTo) {
  EXPECT_EQ(Expr::FromRaw("a").SimilarTo(Expr::FromRaw("a%b")).ToString(),
            "a SIMILAR TO a%b");
}

TEST(Expr, NotSimilarTo) {
  EXPECT_EQ(Expr::FromRaw("a").NotSimilarTo(Expr::FromRaw("a%b")).ToString(),
            "a NOT SIMILAR TO a%b");
}

TEST(Expr, In) {
  EXPECT_EQ(Expr::FromRaw("a")
                .In(From(Table::FromRaw("foo")).Select(Expr::FromRaw("bar")))
                .ToString(),
            "a IN (SELECT bar FROM foo)");
}

TEST(Expr, NotIn) {
  EXPECT_EQ(Expr::FromRaw("a")
                .NotIn(From(Table::FromRaw("foo")).Select(Expr::FromRaw("bar")))
                .ToString(),
            "a NOT IN (SELECT bar FROM foo)");
}

TEST(Expr, InValueList) {
  EXPECT_EQ(Expr::FromRaw("a").In({1, 2, 3}).ToString(), "a IN (1, 2, 3)");
  EXPECT_EQ(Expr::FromRaw("a").In({_1}).ToString(), "a IN ($1)");
  EXPECT_EQ(Expr::FromRaw("a").NotIn({1, 2}).ToString(), "a NOT IN (1, 2)");
}

TEST(Expr, InEmptyListThrows) {
  // "a IN ()" is not valid SQL, so an empty list can only be a caller bug.
  EXPECT_THROW(Ignore(Expr::FromRaw("a").In({})), std::invalid_argument);
  EXPECT_THROW(Ignore(Expr::FromRaw("a").NotIn({})), std::invalid_argument);
}

TEST(Expr, EqAnyNeAll) {
  EXPECT_EQ(Expr::FromRaw("a").EqAny(_1).ToString(), "a = ANY ($1)");
  EXPECT_EQ(Expr::FromRaw("a").NeAll(_1).ToString(), "a <> ALL ($1)");
  EXPECT_EQ(Expr::FromRaw("a")
                .EqAny(From(Table::FromRaw("foo")).Select(Expr::FromRaw("bar")))
                .ToString(),
            "a = ANY (SELECT bar FROM foo)");
  EXPECT_EQ(Expr::FromRaw("a")
                .NeAll(From(Table::FromRaw("foo")).Select(Expr::FromRaw("bar")))
                .ToString(),
            "a <> ALL (SELECT bar FROM foo)");
}

TEST(Expr, EqAnyPrecedence) {
  EXPECT_EQ(
      (Expr::FromRaw("a").EqAny(_1) && Condition::FromRaw("b")).ToString(),
      "a = ANY ($1) AND b");
}

TEST(Expr, Is) {
  EXPECT_EQ(Expr::FromRaw("a").IsTrue().ToString(), "a IS TRUE");
  EXPECT_EQ(Expr::FromRaw("a").IsFalse().ToString(), "a IS FALSE");
  EXPECT_EQ(Expr::FromRaw("a").IsNull().ToString(), "a IS NULL");
  EXPECT_EQ(Expr::FromRaw("a").IsNotNull().ToString(), "a IS NOT NULL");
}

TEST(Expr, IsDistinctFrom) {
  EXPECT_EQ(Expr::FromRaw("a").IsDistinctFrom(Expr::FromRaw("b")).ToString(),
            "a IS DISTINCT FROM b");
  EXPECT_EQ(Expr::FromRaw("a").IsNotDistinctFrom(Expr::FromRaw("b")).ToString(),
            "a IS NOT DISTINCT FROM b");
}

TEST(Condition, LogicalAndEmbedding) {
  // Conditions compose without going back through Expr.
  EXPECT_EQ(
      (Expr::FromRaw("a").IsTrue() && Expr::FromRaw("b").IsNull()).ToString(),
      "a IS TRUE AND b IS NULL");

  // A bare boolean column is not implicitly a Condition; the idiomatic
  // escape hatch is IsTrue()/IsFalse()/IsNotNull().
  Table tbl = Table::FromRaw("test");
  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .Where(Expr::FromRaw("is_active").IsTrue())
                .ToString(),
            "SELECT * FROM test WHERE is_active IS TRUE");

  // A Condition can still be embedded as a value expression, e.g. in a
  // SELECT list.
  EXPECT_EQ(From(tbl).Select(Expr::FromRaw("a") == 1).ToString(),
            "SELECT a = 1 FROM test");
}

TEST(Expr, Compare) {
  EXPECT_EQ((Expr(1) <= 2).ToString(), "1 <= 2");
  EXPECT_EQ((Expr(1) >= 2).ToString(), "1 >= 2");
  EXPECT_EQ((Expr(1) != 2).ToString(), "1 != 2");
}

TEST(Expr, Null) {
  EXPECT_EQ(Expr::Null().ToString(), "NULL");
  EXPECT_EQ(Update(Table::FromRaw("foo"))
                .Set(Expr::FromRaw("a"), Expr::Null())
                .ToString(),
            "UPDATE foo SET a = NULL");
}

TEST(Expr, Bool) {
  EXPECT_EQ(Expr::Bool(true).ToString(), "TRUE");
  EXPECT_EQ(Expr::Bool(false).ToString(), "FALSE");
}

TEST(Expr, IntegerLiterals) {
  EXPECT_EQ(Expr(0).ToString(), "0");
  EXPECT_EQ(Expr(-42).ToString(), "-42");
  EXPECT_EQ(Expr(std::numeric_limits<std::int64_t>::min()).ToString(),
            "-9223372036854775808");
  EXPECT_EQ(Expr(std::numeric_limits<std::uint64_t>::max()).ToString(),
            "18446744073709551615");
}

TEST(Expr, DoubleLiterals) {
  // std::to_string would render these as "0.100000" and "1.000000".
  EXPECT_EQ(Expr(0.1).ToString(), "0.1");
  EXPECT_EQ(Expr(1.0).ToString(), "1.0");
  EXPECT_EQ(Expr(-2.5F).ToString(), "-2.5");
  EXPECT_EQ(Expr(1e300).ToString(), "1e+300");
}

TEST(Expr, NonFiniteDoubleThrows) {
  // Named locals: "Expr(std::numeric_limits<double>::infinity())" would be
  // parsed as a declaration, not a call.
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();

  EXPECT_THROW(Expr{nan}.ToString(), std::invalid_argument);
  EXPECT_THROW(Expr{inf}.ToString(), std::invalid_argument);
}

TEST(Expr, Literal) {
  EXPECT_EQ(Expr::Literal("x").ToString(), "'x'");
  EXPECT_EQ(Expr::Literal("it's").ToString(), "'it''s'");
  EXPECT_EQ(Expr::Literal("").ToString(), "''");
}

TEST(Expr, Ident) {
  EXPECT_EQ(Expr::Ident("name").ToString(), "\"name\"");
  EXPECT_EQ(Expr::Ident("weird\"name").ToString(), "\"weird\"\"name\"");
}

TEST(Expr, Call) {
  EXPECT_EQ(Expr::Call("COALESCE", {Expr::FromRaw("a"), Expr::FromRaw("b")})
                .ToString(),
            "COALESCE(a, b)");
  EXPECT_EQ(Expr::Call("NOW", {}).ToString(), "NOW()");
  EXPECT_EQ(Expr::Call("ABS", {Expr(1) - 2}).ToString(), "ABS(1 - 2)");
  EXPECT_EQ(Expr::Call("pg_catalog.now", {}).ToString(), "pg_catalog.now()");
}

TEST(Expr, CallInvalidNameThrows) {
  EXPECT_THROW(Ignore(Expr::Call("not an identifier", {})),
               std::invalid_argument);
  EXPECT_THROW(Ignore(Expr::Call("", {})), std::invalid_argument);
}

TEST(Expr, CallDistinct) {
  EXPECT_EQ(
      Expr::CallDistinct("string_agg", {Expr::FromRaw("x"), Expr::Literal(",")})
          .ToString(),
      "string_agg(DISTINCT x, ',')");
}

TEST(Expr, CallDistinctInvalidThrows) {
  EXPECT_THROW(Ignore(Expr::CallDistinct("not an identifier", {Expr(1)})),
               std::invalid_argument);
  EXPECT_THROW(Ignore(Expr::CallDistinct("string_agg", {})),
               std::invalid_argument);
}

TEST(Expr, BinaryOp) {
  EXPECT_EQ(Expr::FromRaw("a").BinaryOp("~", Expr::Literal("^x")).ToString(),
            "a ~ '^x'");
  EXPECT_EQ(Expr::FromRaw("a").BinaryOp("@>", Expr::FromRaw("b")).ToString(),
            "a @> b");
  // kAnyOther default precedence binds looser than kAnd, so an AND operand
  // gets parenthesized.
  EXPECT_EQ((Expr::FromRaw("a") == 1)
                .operator Expr()
                .BinaryOp("&&", Expr::FromRaw("b"))
                .ToString(),
            "(a = 1) && b");
  EXPECT_EQ(Expr::FromRaw("a")
                .BinaryOp("<<", Expr::FromRaw("b"), OperatorPrecedence::kExp)
                .Dot(Expr::FromRaw("c"))
                .ToString(),
            "(a << b).c");
}

TEST(Expr, BinaryOpInvalidNameThrows) {
  EXPECT_THROW(Ignore(Expr::FromRaw("a").BinaryOp("", Expr::FromRaw("b"))),
               std::invalid_argument);
  EXPECT_THROW(Ignore(Expr::FromRaw("a").BinaryOp("x", Expr::FromRaw("b"))),
               std::invalid_argument);
  EXPECT_THROW(Ignore(Expr::FromRaw("a").BinaryOp("--", Expr::FromRaw("b"))),
               std::invalid_argument);
  EXPECT_THROW(Ignore(Expr::FromRaw("a").BinaryOp("/*", Expr::FromRaw("b"))),
               std::invalid_argument);
  EXPECT_THROW(Ignore(Expr::FromRaw("a").BinaryOp("++", Expr::FromRaw("b"))),
               std::invalid_argument);
}

TEST(Expr, PrefixOp) {
  EXPECT_EQ(Expr::PrefixOp("@", Expr::FromRaw("x")).ToString(), "@ x");
  EXPECT_EQ(Expr::PrefixOp("~", Expr::FromRaw("x"))
                .Dot(Expr::FromRaw("y"))
                .ToString(),
            "(~ x).y");
}

TEST(Expr, PrefixOpInvalidNameThrows) {
  EXPECT_THROW(Ignore(Expr::PrefixOp("", Expr::FromRaw("x"))),
               std::invalid_argument);
  EXPECT_THROW(Ignore(Expr::PrefixOp("x", Expr::FromRaw("x"))),
               std::invalid_argument);
}

TEST(Expr, Case) {
  EXPECT_EQ(Case()
                .When(Expr::FromRaw("a") == 1)
                .Then(Expr::FromRaw("x"))
                .End()
                .ToString(),
            "CASE WHEN a = 1 THEN x END");
}

TEST(Expr, CaseMultiWhenElse) {
  EXPECT_EQ(Case()
                .When(Expr::FromRaw("a") == 1)
                .Then(Expr::FromRaw("x"))
                .When(Expr::FromRaw("a") == 2)
                .Then(Expr::FromRaw("y"))
                .Else(Expr::FromRaw("z"))
                .End()
                .ToString(),
            "CASE WHEN a = 1 THEN x WHEN a = 2 THEN y ELSE z END");
}

TEST(Expr, CaseThenWithoutWhenThrows) {
  EXPECT_THROW(Ignore(Case().Then(Expr::FromRaw("x"))), std::logic_error);
}

TEST(Expr, CaseEndWithoutWhenThrows) {
  EXPECT_THROW(Ignore(Case().End()), std::logic_error);
}

TEST(Expr, Exists) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(Expr::Exists(From(tbl).Select(Expr::FromRaw("1"))).ToString(),
            "EXISTS (SELECT 1 FROM foo)");
  EXPECT_EQ(Expr::NotExists(From(tbl).Select(Expr::FromRaw("1"))).ToString(),
            "NOT EXISTS (SELECT 1 FROM foo)");
}

TEST(Expr, Aggregates) {
  Column age{"age", "BIGINT"};

  EXPECT_EQ(Expr::Count(age).ToString(), "COUNT(age)");
  EXPECT_EQ(Expr::CountAll().ToString(), "COUNT(*)");
  EXPECT_EQ(Expr::CountDistinct(age).ToString(), "COUNT(DISTINCT age)");
  EXPECT_EQ(Expr::Sum(age).ToString(), "SUM(age)");
  EXPECT_EQ(Expr::Avg(age).ToString(), "AVG(age)");
  EXPECT_EQ(Expr::Min(age).ToString(), "MIN(age)");
  EXPECT_EQ(Expr::Max(age).ToString(), "MAX(age)");
}

TEST(Expr, Coalesce) {
  EXPECT_EQ(Expr::Coalesce({Expr::FromRaw("a"), Expr::Null(), 0}).ToString(),
            "COALESCE(a, NULL, 0)");
}

TEST(Expr, CoalesceEmptyThrows) {
  EXPECT_THROW(Ignore(Expr::Coalesce({})), std::invalid_argument);
}

TEST(Expr, NullIf) {
  EXPECT_EQ(Expr::NullIf(Expr::FromRaw("a"), Expr::FromRaw("b")).ToString(),
            "NULLIF(a, b)");
}

TEST(Expr, Greatest) {
  EXPECT_EQ(
      Expr::Greatest({Expr::FromRaw("a"), Expr::FromRaw("b"), 0}).ToString(),
      "GREATEST(a, b, 0)");
}

TEST(Expr, GreatestEmptyThrows) {
  EXPECT_THROW(Ignore(Expr::Greatest({})), std::invalid_argument);
}

TEST(Expr, Least) {
  EXPECT_EQ(Expr::Least({Expr::FromRaw("a"), Expr::FromRaw("b"), 0}).ToString(),
            "LEAST(a, b, 0)");
}

TEST(Expr, LeastEmptyThrows) {
  EXPECT_THROW(Ignore(Expr::Least({})), std::invalid_argument);
}

TEST(Expr, LiteralInjectionAttempt) {
  // Even a hostile value can't break out of the quoted literal.
  EXPECT_EQ(Expr::FromRaw("x")
                .In({Expr::Literal("'; DROP TABLE users; --")})
                .ToString(),
            "x IN ('''; DROP TABLE users; --')");
}
