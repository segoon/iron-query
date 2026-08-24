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

TEST(Select, Where) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .Where(Expr::FromRaw("a = b"))
                .ToString(),
            "SELECT * FROM test WHERE a = b");
}

TEST(Select, OrderBy) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl).Select(Expr::FromRaw("*")).OrderByRaw("name").ToString(),
            "SELECT * FROM test ORDER BY name");
}

TEST(Select, OrderByMulti) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .OrderByRaw({"name", "age"})
                .ToString(),
            "SELECT * FROM test ORDER BY name, age");
}

TEST(Select, Full) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("a, b"))
                .Where(Expr::FromRaw("a = b"))
                .OrderByRaw("name")
                .ToString(),
            "SELECT a, b FROM test WHERE a = b ORDER BY name");
}

TEST(Select, GroupByHaving) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("a, COUNT(*)"))
                .GroupByRaw("a")
                .Having(Expr::FromRaw("COUNT(*)") > 1)
                .ToString(),
            "SELECT a, COUNT(*) FROM test GROUP BY a HAVING COUNT(*) > 1");
}

TEST(Select, GroupByMulti) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(
      From(tbl).Select(Expr::FromRaw("*")).GroupByRaw({"a", "b"}).ToString(),
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
                .Where(Expr::FromRaw("a > 0"))
                .GroupByRaw("a")
                .Having(Expr::FromRaw("COUNT(*) > 1"))
                .OrderByRaw("a")
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
                .Where(Expr::FromRaw("a = b"))
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
                .GroupByRaw({"a", "b"})
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
                .OrderByRaw({"name", "age"})
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

  EXPECT_EQ(DeleteFrom(tbl).Where(Expr::FromRaw("a = 1")).ToString(),
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

  EXPECT_EQ(
      Join(tbl1, tbl2, Cross()).On(Expr::FromRaw("foo.a = bar.b")).ToString(),
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

  EXPECT_EQ(WithRaw("cte", From(tbl).Select(Expr::FromRaw("a")))
                .Main(From(Table::FromRaw("cte")).Select(Expr::FromRaw("*")))
                .ToString(),
            "WITH cte AS (SELECT a FROM foo) SELECT * FROM cte");
}

TEST(With, Chained) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(WithRaw("a", From(tbl).Select(Expr::FromRaw("x")))
                .WithRaw("b", From(tbl).Select(Expr::FromRaw("y")))
                .Main(From(Table::FromRaw("a")).Select(Expr::FromRaw("*")))
                .ToString(),
            "WITH a AS (SELECT x FROM foo), b AS (SELECT y FROM foo) "
            "SELECT * FROM a");
}

TEST(With, UsableAsSubquery) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(
      Expr::FromRaw("x")
          .In(WithRaw("cte", From(tbl).Select(Expr::FromRaw("a")))
                  .Main(From(Table::FromRaw("cte")).Select(Expr::FromRaw("*"))))
          .ToString(),
      "x IN (WITH cte AS (SELECT a FROM foo) SELECT * FROM cte)");
}

TEST(Join, SelectSelect) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(Join(From(tbl).Select(Expr::FromRaw("a")),
                 From(tbl).Select(Expr::FromRaw("b")), Cross())
                .On(Expr::FromRaw("a = b"))
                .ToString(),
            "(SELECT a FROM foo) CROSS JOIN (SELECT b FROM foo) ON a = b");
}

TEST(Insert, Basic) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .Values({1, 2})
                .ToString(),
            "INSERT INTO foo (a, b) VALUES (1, 2)");
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
  EXPECT_THROW(Update(tbl).Where(Expr::FromRaw("a = 1")).ToString(),
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
            "SELECT bar.name FROM (foo AS bar)");
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

TEST(Expr, symbol) {
  EXPECT_EQ(Expr::FromRaw("x").ToString(), "x");
  EXPECT_EQ(Expr::FromRaw(std::string("x")).ToString(), "x");
  EXPECT_EQ(Expr(1).ToString(), "1");
}

TEST(Expr, Logical) {
  EXPECT_EQ(((Expr(1) < 2) && (Expr(2) == Expr::FromRaw("age"))).ToString(),
            "1 < 2 AND 2 = age");
  EXPECT_EQ(((Expr::FromRaw("x") && Expr::FromRaw("y")) || Expr::FromRaw("z"))
                .ToString(),
            "x AND y OR z");
  EXPECT_EQ(((Expr::FromRaw("x") || Expr::FromRaw("y")) && Expr::FromRaw("z"))
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
  EXPECT_EQ(Expr::FromRaw("a").CollateRaw(R"("C")").ToString(),
            R"(a COLLATE "C")");
}

TEST(Expr, Index) {
  EXPECT_EQ(Expr::FromRaw("a")[Expr::FromRaw("b")].ToString(), "a[b]");
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
  EXPECT_EQ(Expr::CallRaw("COALESCE", {Expr::FromRaw("a"), Expr::FromRaw("b")})
                .ToString(),
            "COALESCE(a, b)");
  EXPECT_EQ(Expr::CallRaw("NOW", {}).ToString(), "NOW()");
  EXPECT_EQ(Expr::CallRaw("ABS", {Expr(1) - 2}).ToString(), "ABS(1 - 2)");
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
