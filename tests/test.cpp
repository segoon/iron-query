#include <gtest/gtest.h>

#include <iron_query/iron_query.hpp>

using namespace iron_query;

TEST(Select, From) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl).Select(Expr::FromRaw("*")).ToString(),
            "SELECT * FROM test");
}

TEST(Select, SelectSmthFrom) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl).Select(Expr::FromRaw("a, b")).ToString(),
            "SELECT a, b FROM test");
}

TEST(Select, Multi) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(
      From(tbl).Select({Expr::FromRaw("a"), Expr::FromRaw("b")}).ToString(),
      "SELECT a, b FROM test");
}

TEST(Select, ClauseListsReplacePreviousLists) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select({Expr::FromRaw("a")})
                .Select({Expr::FromRaw("b")})
                .GroupBy({Expr::FromRaw("a")})
                .GroupBy({Expr::FromRaw("b")})
                .OrderBy({Expr::FromRaw("a")})
                .OrderBy({Expr::FromRaw("b")})
                .ToString(),
            "SELECT b FROM test GROUP BY b ORDER BY b");
}

TEST(Select, Where) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .Where(Condition::FromRaw("a = b"))
                .ToString(),
            "SELECT * FROM test WHERE a = b");
}

TEST(Select, OrderBy) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .OrderBy(Expr::FromRaw("name"))
                .ToString(),
            "SELECT * FROM test ORDER BY name");
}

TEST(Select, OrderByMulti) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .OrderBy({Expr::FromRaw("name"), Expr::FromRaw("age")})
                .ToString(),
            "SELECT * FROM test ORDER BY name, age");
}

TEST(Select, OrderByDescending) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .OrderBy({Expr::FromRaw("name"),
                          {Expr::FromRaw("age"), SortDirection::kDescending}})
                .ToString(),
            "SELECT * FROM test ORDER BY name, age DESC");
}

TEST(Select, Full) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("a, b"))
                .Where(Condition::FromRaw("a = b"))
                .OrderBy(Expr::FromRaw("name"))
                .ToString(),
            "SELECT a, b FROM test WHERE a = b ORDER BY name");
}

TEST(Select, GroupByHaving) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("a, COUNT(*)"))
                .GroupBy(Expr::FromRaw("a"))
                .Having(Expr::FromRaw("COUNT(*)") > 1)
                .ToString(),
            "SELECT a, COUNT(*) FROM test GROUP BY a HAVING COUNT(*) > 1");
}

TEST(Select, GroupByMulti) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .GroupBy({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .ToString(),
            "SELECT * FROM test GROUP BY a, b");
}

TEST(Select, LimitOffset) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl).Select(Expr::FromRaw("*")).Limit(10).Offset(5).ToString(),
            "SELECT * FROM test LIMIT 10 OFFSET 5");
}

TEST(Select, AllClauses) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("a"))
                .Where(Condition::FromRaw("a > 0"))
                .GroupBy(Expr::FromRaw("a"))
                .Having(Condition::FromRaw("COUNT(*) > 1"))
                .OrderBy(Expr::FromRaw("a"))
                .Limit(10)
                .Offset(5)
                .ToString(),
            "SELECT a FROM test WHERE a > 0 GROUP BY a HAVING COUNT(*) > 1 "
            "ORDER BY a LIMIT 10 OFFSET 5");
}

TEST(Select, MissingSelectThrows) {
  Table tbl = Table::FromRaw("test");
  SelectExpr s = From(tbl);
  EXPECT_THROW(s.ToString(), std::logic_error);
}

TEST(SelectFormatted, Basic) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl).Select(Expr::FromRaw("*")).ToStringFormatted(),
            "SELECT\n"
            "    *\n"
            "FROM\n"
            "    test");
}

TEST(SelectFormatted, MultiSelect) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .ToStringFormatted(),
            "SELECT\n"
            "    a,\n"
            "    b\n"
            "FROM\n"
            "    test");
}

TEST(SelectFormatted, Where) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .Where(Condition::FromRaw("a = b"))
                .ToStringFormatted(),
            "SELECT\n"
            "    *\n"
            "FROM\n"
            "    test\n"
            "WHERE\n"
            "    a = b");
}

TEST(SelectFormatted, GroupByHaving) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("a"))
                .GroupBy({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .Having(Expr::FromRaw("COUNT(*)") > 1)
                .ToStringFormatted(),
            "SELECT\n"
            "    a\n"
            "FROM\n"
            "    test\n"
            "GROUP BY\n"
            "    a,\n"
            "    b\n"
            "HAVING\n"
            "    COUNT(*) > 1");
}

TEST(SelectFormatted, OrderByMulti) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .OrderBy({Expr::FromRaw("name"), Expr::FromRaw("age")})
                .ToStringFormatted(),
            "SELECT\n"
            "    *\n"
            "FROM\n"
            "    test\n"
            "ORDER BY\n"
            "    name,\n"
            "    age");
}

TEST(SelectFormatted, LimitOffset) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .Limit(10)
                .Offset(5)
                .ToStringFormatted(),
            "SELECT\n"
            "    *\n"
            "FROM\n"
            "    test\n"
            "LIMIT\n"
            "    10\n"
            "OFFSET\n"
            "    5");
}

TEST(SelectFormatted, MissingSelectThrows) {
  Table tbl = Table::FromRaw("test");
  SelectExpr s = From(tbl);
  EXPECT_THROW(s.ToStringFormatted(), std::logic_error);
}

TEST(Delete, From) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(DeleteFrom(tbl).ToString(), "DELETE FROM test");
}

TEST(Delete, Where) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(DeleteFrom(tbl).Where(Condition::FromRaw("a = 1")).ToString(),
            "DELETE FROM test WHERE a = 1");
}

TEST(Join, Inner) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, Inner()).ToString(), "foo INNER JOIN bar");
}

TEST(Join, Cross) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, Cross()).ToString(), "foo CROSS JOIN bar");
}

TEST(Join, On) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, Cross())
                .On(Condition::FromRaw("foo.a = bar.b"))
                .ToString(),
            "foo CROSS JOIN bar ON foo.a = bar.b");
}

TEST(Join, LeftOuter) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, LeftOuter()).ToString(),
            "foo LEFT OUTER JOIN bar");
}

TEST(Join, RightOuter) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, RightOuter()).ToString(),
            "foo RIGHT OUTER JOIN bar");
}

TEST(Join, FullOuter) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, FullOuter()).ToString(),
            "foo FULL OUTER JOIN bar");
}

TEST(SetOp, Union) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(SetOp(From(tbl).Select(Expr::FromRaw("a")),
                  From(tbl).Select(Expr::FromRaw("b")), Union())
                .ToString(),
            "(SELECT a FROM foo) UNION (SELECT b FROM foo)");
}

TEST(SetOp, UnionAll) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(SetOp(From(tbl).Select(Expr::FromRaw("a")),
                  From(tbl).Select(Expr::FromRaw("b")), UnionAll())
                .ToString(),
            "(SELECT a FROM foo) UNION ALL (SELECT b FROM foo)");
}

TEST(SetOp, Intersect) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(SetOp(From(tbl).Select(Expr::FromRaw("a")),
                  From(tbl).Select(Expr::FromRaw("b")), Intersect())
                .ToString(),
            "(SELECT a FROM foo) INTERSECT (SELECT b FROM foo)");
}

TEST(SetOp, Except) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(SetOp(From(tbl).Select(Expr::FromRaw("a")),
                  From(tbl).Select(Expr::FromRaw("b")), Except())
                .ToString(),
            "(SELECT a FROM foo) EXCEPT (SELECT b FROM foo)");
}

TEST(SetOp, UsableAsSubquery) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(Expr::FromRaw("x")
                .In(SetOp(From(tbl).Select(Expr::FromRaw("a")),
                          From(tbl).Select(Expr::FromRaw("b")), Union()))
                .ToString(),
            "x IN ((SELECT a FROM foo) UNION (SELECT b FROM foo))");
}

TEST(With, Single) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(With("cte", From(tbl).Select(Expr::FromRaw("a")))
                .Main(From(Table::FromRaw("cte")).Select(Expr::FromRaw("*")))
                .ToString(),
            "WITH cte AS (SELECT a FROM foo) SELECT * FROM cte");
}

TEST(With, Chained) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(With("a", From(tbl).Select(Expr::FromRaw("x")))
                .With("b", From(tbl).Select(Expr::FromRaw("y")))
                .Main(From(Table::FromRaw("a")).Select(Expr::FromRaw("*")))
                .ToString(),
            "WITH a AS (SELECT x FROM foo), b AS (SELECT y FROM foo) "
            "SELECT * FROM a");
}

TEST(With, UsableAsSubquery) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(
      Expr::FromRaw("x")
          .In(With("cte", From(tbl).Select(Expr::FromRaw("a")))
                  .Main(From(Table::FromRaw("cte")).Select(Expr::FromRaw("*"))))
          .ToString(),
      "x IN (WITH cte AS (SELECT a FROM foo) SELECT * FROM cte)");
}

TEST(With, InvalidNameThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(With("not an identifier", From(tbl).Select(Expr::FromRaw("a"))),
               std::invalid_argument);
}

TEST(With, DottedNameAllowed) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(With("my_schema.cte", From(tbl).Select(Expr::FromRaw("a")))
                .Main(From(tbl).Select(Expr::FromRaw("*")))
                .ToString(),
            "WITH my_schema.cte AS (SELECT a FROM foo) SELECT * FROM foo");
}

TEST(Join, SelectSelect) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(Join(From(tbl).Select(Expr::FromRaw("a")),
                 From(tbl).Select(Expr::FromRaw("b")), Cross())
                .On(Condition::FromRaw("a = b"))
                .ToString(),
            "(SELECT a FROM foo) CROSS JOIN (SELECT b FROM foo) ON a = b");
}

// PostgreSQL attaches a table alias directly to the FROM item; parentheses are
// only ever allowed around a bare joined_table.
class AliasTest : public ::testing::Test {
protected:
  const Table foo = Table::FromRaw("foo");
  const Table bar = Table::FromRaw("bar");
};

TEST_F(AliasTest, TableIsNotParenthesized) {
  EXPECT_EQ(foo.As("t").ToString(), "foo AS t");
  EXPECT_EQ(From(foo.As("t")).Select(Expr::FromRaw("*")).ToString(),
            "SELECT * FROM foo AS t");
}

TEST_F(AliasTest, SubqueryKeepsExactlyOnePairOfParens) {
  EXPECT_EQ(From(foo).Select(Expr::FromRaw("a")).As("s").ToString(),
            "(SELECT a FROM foo) AS s");
}

TEST_F(AliasTest, JoinIsParenthesized) {
  EXPECT_EQ(Join(foo, bar, Inner()).As("j").ToString(),
            "(foo INNER JOIN bar) AS j");
}

TEST_F(AliasTest, InvalidNameThrows) {
  EXPECT_THROW(foo.As("not an identifier"), std::invalid_argument);
  EXPECT_THROW(foo.As(""), std::invalid_argument);
}

TEST(Insert, Basic) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .Values({1, 2})
                .ToString(),
            "INSERT INTO foo (a, b) VALUES (1, 2)");
}

TEST(Insert, ColumnsAndValuesReplacePreviousLists) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a")})
                .Columns({Expr::FromRaw("b")})
                .Values({1})
                .Values({2})
                .ToString(),
            "INSERT INTO foo (b) VALUES (2)");
}

TEST(Insert, ArityMismatchThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(InsertInto(tbl)
                   .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                   .Values({1}),
               std::logic_error);
  EXPECT_THROW(InsertInto(tbl).Values({1}).Columns(
                   {Expr::FromRaw("a"), Expr::FromRaw("b")}),
               std::logic_error);
}

TEST(Insert, MissingColumnsThrows) {
  Table tbl = Table::FromRaw("foo");
  EXPECT_THROW(InsertInto(tbl).Values({1, 2}).ToString(), std::logic_error);
}

TEST(Insert, MissingValuesThrows) {
  Table tbl = Table::FromRaw("foo");
  EXPECT_THROW(InsertInto(tbl)
                   .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                   .ToString(),
               std::logic_error);
}

TEST(Update, Basic) {
  Table tbl = Table::FromRaw("foo");
  Column age{"age", "BIGINT"};

  EXPECT_EQ(Update(tbl).Set(age, 42).Where(age == 1).ToString(),
            "UPDATE foo SET age = 42 WHERE age = 1");
}

TEST(Update, MultiSet) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(Update(tbl)
                .Set(Expr::FromRaw("a"), 1)
                .Set(Expr::FromRaw("b"), 2)
                .ToString(),
            "UPDATE foo SET a = 1, b = 2");
}

TEST(Update, MissingSetThrows) {
  Table tbl = Table::FromRaw("foo");
  EXPECT_THROW(Update(tbl).Where(Condition::FromRaw("a = 1")).ToString(),
               std::logic_error);
}

TEST(Column, Select) {
  Table tbl = Table::FromRaw("foo");
  Column name{"name", "TEXT"};
  Column age{"age", "BIGINT"};

  EXPECT_EQ(From(tbl).Select({name, age}).ToString(),
            "SELECT name, age FROM foo");
}

TEST(Column, As) {
  Table tbl = Table::FromRaw("foo");
  Column name{"name", "TEXT"};

  auto bar = "bar";
  EXPECT_EQ(From(tbl.As(bar)).Select(Expr::FromRaw(bar).Dot(name)).ToString(),
            "SELECT bar.name FROM foo AS bar");
}

TEST(Column, Compare) {
  Column age{"age", "BIGINT"};

  EXPECT_EQ((age < 18).ToString(), "age < 18");
  EXPECT_EQ((age <= 18).ToString(), "age <= 18");
  EXPECT_EQ((age > 18).ToString(), "age > 18");
  EXPECT_EQ((age >= 18).ToString(), "age >= 18");
  EXPECT_EQ((age == 18).ToString(), "age = 18");
  EXPECT_EQ((age != 18).ToString(), "age != 18");
}

TEST(TableWithColumns, Simple) {
  Column name{"name", "TEXT"};
  Column age{"age", "BIGINT"};
  TableWithColumns tbl = TableWithColumns::FromRaw("foo", {name, age});

  EXPECT_EQ(From(tbl).Select(tbl.SelectArgAll()).ToString(),
            "SELECT name, age FROM foo");
}

TEST(TableAlias, Dot) {
  TableAlias alias = TableAlias::From("bar");
  Column name{"name", "TEXT"};

  EXPECT_EQ(alias.Dot("name"), "bar.name");
  EXPECT_EQ(alias.Dot(name), "bar.name");
}

TEST(TableAlias, InvalidNameThrows) {
  EXPECT_THROW(TableAlias::From("not an identifier"), std::invalid_argument);
  EXPECT_THROW(TableAlias::From(""), std::invalid_argument);
}

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

TEST(Expr, Is) {
  EXPECT_EQ(Expr::FromRaw("a").IsTrue().ToString(), "a IS TRUE");
  EXPECT_EQ(Expr::FromRaw("a").IsFalse().ToString(), "a IS FALSE");
  EXPECT_EQ(Expr::FromRaw("a").IsNull().ToString(), "a IS NULL");
  EXPECT_EQ(Expr::FromRaw("a").IsNotNull().ToString(), "a IS NOT NULL");
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
  EXPECT_THROW(Expr::Call("not an identifier", {}), std::invalid_argument);
  EXPECT_THROW(Expr::Call("", {}), std::invalid_argument);
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
  EXPECT_THROW(Case().Then(Expr::FromRaw("x")), std::logic_error);
}

TEST(Expr, CaseEndWithoutWhenThrows) {
  EXPECT_THROW(Case().End(), std::logic_error);
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
  EXPECT_EQ(Expr::Sum(age).ToString(), "SUM(age)");
  EXPECT_EQ(Expr::Avg(age).ToString(), "AVG(age)");
  EXPECT_EQ(Expr::Min(age).ToString(), "MIN(age)");
  EXPECT_EQ(Expr::Max(age).ToString(), "MAX(age)");
}

TEST(Expr, LiteralInjectionAttempt) {
  // Even a hostile value can't break out of the quoted literal.
  EXPECT_EQ(Expr::FromRaw("x")
                .In(Expr::Literal("'; DROP TABLE users; --"))
                .ToString(),
            "x IN '''; DROP TABLE users; --'");
}

// Example of autogenerated declarations
// Table "users"
namespace users {

// Fields
const Column kName{"name", "TEXT"};
const Column kAge{"age", "BIGINT"};

// Explicit fields enumeration ("*" replacement)
const std::vector<Column> kAll = {
    kName,
    kAge,
};

// The table itself
const TableWithColumns kTable = TableWithColumns::FromRaw("users", kAll);

// TODO: unable to create table fields with name "table" or "all", hmm...

} // namespace users
