#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

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

TEST(Select, SelectAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(
      Ignore(
          From(tbl).Select({Expr::FromRaw("a")}).Select({Expr::FromRaw("b")})),
      std::logic_error);
}

TEST(Select, GroupByAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(Ignore(From(tbl)
                          .Select({Expr::FromRaw("a")})
                          .GroupBy({Expr::FromRaw("a")})
                          .GroupBy({Expr::FromRaw("b")})),
               std::logic_error);
}

TEST(Select, OrderByAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(Ignore(From(tbl)
                          .Select({Expr::FromRaw("a")})
                          .OrderBy({Expr::FromRaw("a")})
                          .OrderBy({Expr::FromRaw("b")})),
               std::logic_error);
}

TEST(Select, DistinctOnAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(Ignore(From(tbl)
                          .DistinctOn(Expr::FromRaw("a"))
                          .DistinctOn(Expr::FromRaw("b"))),
               std::logic_error);
}

TEST(Select, Where) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .Where(Condition::FromRaw("a = b"))
                .ToString(),
            "SELECT * FROM test WHERE a = b");
}

TEST(Select, WhereAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(Ignore(From(tbl)
                          .Where(Condition::FromRaw("a = b"))
                          .Where(Condition::FromRaw("c = d"))),
               std::logic_error);
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

TEST(Select, OrderByNullsFirstLast) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .Select(Expr::FromRaw("*"))
                .OrderBy({{Expr::FromRaw("name"), SortDirection::kAscending,
                           NullsOrder::kFirst},
                          {Expr::FromRaw("age"), SortDirection::kDescending,
                           NullsOrder::kLast}})
                .ToString(),
            "SELECT * FROM test ORDER BY name NULLS FIRST, age DESC NULLS "
            "LAST");
}

TEST(Select, Distinct) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl).Distinct().Select(Expr::FromRaw("a")).ToString(),
            "SELECT DISTINCT a FROM test");
}

TEST(Select, DistinctOn) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .DistinctOn(Expr::FromRaw("a"))
                .Select(Expr::FromRaw("*"))
                .ToString(),
            "SELECT DISTINCT ON (a) * FROM test");
}

TEST(Select, DistinctOnMulti) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .DistinctOn({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .Select(Expr::FromRaw("*"))
                .ToString(),
            "SELECT DISTINCT ON (a, b) * FROM test");
}

TEST(Select, DistinctOnWithOrderBy) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .DistinctOn(Expr::FromRaw("a"))
                .Select(Expr::FromRaw("*"))
                .OrderBy({Expr::FromRaw("a"), Expr::FromRaw("created_at desc")})
                .ToString(),
            "SELECT DISTINCT ON (a) * FROM test "
            "ORDER BY a, created_at desc");
}

TEST(Select, DistinctAndDistinctOnThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(From(tbl)
                   .Distinct()
                   .DistinctOn(Expr::FromRaw("a"))
                   .Select(Expr::FromRaw("*"))
                   .ToString(),
               std::logic_error);
  EXPECT_THROW(From(tbl)
                   .DistinctOn(Expr::FromRaw("a"))
                   .Distinct()
                   .Select(Expr::FromRaw("*"))
                   .ToString(),
               std::logic_error);
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

TEST(Select, HavingAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(Ignore(From(tbl)
                          .Select(Expr::FromRaw("*"))
                          .Having(Expr::FromRaw("COUNT(*)") > 1)
                          .Having(Expr::FromRaw("COUNT(*)") > 2)),
               std::logic_error);
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

TEST(Select, LimitAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(Ignore(From(tbl).Select(Expr::FromRaw("*")).Limit(10).Limit(20)),
               std::logic_error);
}

TEST(Select, OffsetAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(
      Ignore(From(tbl).Select(Expr::FromRaw("*")).Offset(5).Offset(10)),
      std::logic_error);
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

TEST(SelectFormatted, Distinct) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl).Distinct().Select(Expr::FromRaw("*")).ToStringFormatted(),
            "SELECT DISTINCT\n"
            "    *\n"
            "FROM\n"
            "    test");
}

TEST(SelectFormatted, DistinctOn) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(From(tbl)
                .DistinctOn({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .Select(Expr::FromRaw("*"))
                .ToStringFormatted(),
            "SELECT DISTINCT ON (a, b)\n"
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

TEST(Delete, Using) {
  Table tbl = Table::FromRaw("foo");
  Table other = Table::FromRaw("bar");

  EXPECT_EQ(DeleteFrom(tbl)
                .Using(other)
                .Where(Condition::FromRaw("foo.id = bar.id"))
                .ToString(),
            "DELETE FROM foo USING bar WHERE foo.id = bar.id");
}

TEST(Delete, UsingJoin) {
  Table tbl = Table::FromRaw("foo");
  Table a = Table::FromRaw("a");
  Table b = Table::FromRaw("b");

  EXPECT_EQ(
      DeleteFrom(tbl)
          .Using(Join(a, b, Inner()).On(Condition::FromRaw("a.id = b.id")))
          .ToString(),
      "DELETE FROM foo USING a INNER JOIN b ON a.id = b.id");
}

TEST(Delete, UsingAliasedSubquery) {
  Table tbl = Table::FromRaw("foo");
  Table bar = Table::FromRaw("bar");

  EXPECT_EQ(DeleteFrom(tbl)
                .Using(From(bar).Select(Expr::FromRaw("*")).As("s"))
                .ToString(),
            "DELETE FROM foo USING (SELECT * FROM bar) AS s");
}

TEST(Delete, UsingUnaliasedSubqueryThrows) {
  Table tbl = Table::FromRaw("foo");
  Table bar = Table::FromRaw("bar");

  EXPECT_THROW(
      Ignore(DeleteFrom(tbl).Using(From(bar).Select(Expr::FromRaw("*")))),
      std::logic_error);
}

TEST(Delete, UsingAlreadySetThrows) {
  Table tbl = Table::FromRaw("foo");
  Table a = Table::FromRaw("a");
  Table b = Table::FromRaw("b");

  EXPECT_THROW(Ignore(DeleteFrom(tbl).Using(a).Using(b)), std::logic_error);
}

TEST(Delete, Where) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(DeleteFrom(tbl).Where(Condition::FromRaw("a = 1")).ToString(),
            "DELETE FROM test WHERE a = 1");
}

TEST(Delete, WhereAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(Ignore(DeleteFrom(tbl)
                          .Where(Condition::FromRaw("a = 1"))
                          .Where(Condition::FromRaw("b = 2"))),
               std::logic_error);
}

TEST(Delete, Returning) {
  Table tbl = Table::FromRaw("test");

  EXPECT_EQ(DeleteFrom(tbl).Returning(Expr::FromRaw("id")).ToString(),
            "DELETE FROM test RETURNING id");
  EXPECT_EQ(DeleteFrom(tbl)
                .Where(Condition::FromRaw("a = 1"))
                .Returning({Expr::FromRaw("id"), Expr::FromRaw("name")})
                .ToString(),
            "DELETE FROM test WHERE a = 1 RETURNING id, name");
}

TEST(Delete, ReturningAlreadySetThrows) {
  Table tbl = Table::FromRaw("test");

  EXPECT_THROW(Ignore(DeleteFrom(tbl)
                          .Returning(Expr::FromRaw("id"))
                          .Returning(Expr::FromRaw("name"))),
               std::logic_error);
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

TEST(Join, NaturalInner) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, NaturalInner()).ToString(),
            "foo NATURAL INNER JOIN bar");
}

TEST(Join, NaturalLeftOuter) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, NaturalLeftOuter()).ToString(),
            "foo NATURAL LEFT OUTER JOIN bar");
}

TEST(Join, NaturalRightOuter) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, NaturalRightOuter()).ToString(),
            "foo NATURAL RIGHT OUTER JOIN bar");
}

TEST(Join, NaturalFullOuter) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, NaturalFullOuter()).ToString(),
            "foo NATURAL FULL OUTER JOIN bar");
}

TEST(Join, NaturalWithOnThrows) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_THROW(Ignore(Join(tbl1, tbl2, NaturalInner())
                          .On(Condition::FromRaw("foo.a = bar.a"))),
               std::logic_error);
}

TEST(Join, OnAlreadySetThrows) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_THROW(Ignore(Join(tbl1, tbl2, Inner())
                          .On(Condition::FromRaw("foo.a = bar.a"))
                          .On(Condition::FromRaw("foo.b = bar.b"))),
               std::logic_error);
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

  EXPECT_THROW(
      Ignore(With("not an identifier", From(tbl).Select(Expr::FromRaw("a")))),
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
  EXPECT_THROW(Ignore(foo.As("not an identifier")), std::invalid_argument);
  EXPECT_THROW(Ignore(foo.As("")), std::invalid_argument);
}

// A join is a FROM source in its own right; a subquery is one only once it has
// been given an alias.
class FromSourceTest : public ::testing::Test {
protected:
  const Table foo = Table::FromRaw("foo");
  const Table bar = Table::FromRaw("bar");
  const Expr star = Expr::FromRaw("*");
};

TEST_F(FromSourceTest, Join) {
  // No parentheses: PostgreSQL's table_ref allows "( joined_table )" only when
  // an alias follows.
  EXPECT_EQ(
      From(Join(foo, bar, Inner()).On(Condition::FromRaw("foo.a = bar.b")))
          .Select(star)
          .Where(Condition::FromRaw("foo.c > 0"))
          .ToString(),
      "SELECT * FROM foo INNER JOIN bar ON foo.a = bar.b "
      "WHERE foo.c > 0");
}

TEST_F(FromSourceTest, AliasedJoin) {
  EXPECT_EQ(From(Join(foo, bar, Inner()).As("j")).Select(star).ToString(),
            "SELECT * FROM (foo INNER JOIN bar) AS j");
}

TEST_F(FromSourceTest, AliasedSubquery) {
  EXPECT_EQ(From(From(foo).Select(star).As("s")).Select(star).ToString(),
            "SELECT * FROM (SELECT * FROM foo) AS s");
}

TEST_F(FromSourceTest, UnaliasedSubqueryThrows) {
  // PostgreSQL: "subquery in FROM must have an alias".
  EXPECT_THROW(Ignore(From(From(foo).Select(star))), std::logic_error);
  EXPECT_THROW(Ignore(From(SetOp(From(foo).Select(star), From(bar).Select(star),
                                 Union()))),
               std::logic_error);
}

TEST(Insert, Basic) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .Values({1, 2})
                .ToString(),
            "INSERT INTO foo (a, b) VALUES (1, 2)");
}

TEST(Insert, BindParameterValues) {
  // Values() takes plain Expr, so bind placeholders already work with no
  // extra API: $N is just an Expr like any other.
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .Values({_1, _2})
                .ToString(),
            "INSERT INTO foo (a, b) VALUES ($1, $2)");
}

TEST(Insert, MultiRowValues) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                .Rows({{1, 2}, {3, 4}})
                .ToString(),
            "INSERT INTO foo (a, b) VALUES (1, 2), (3, 4)");
}

TEST(Insert, MultiRowValuesSingleColumn) {
  // A single-column row, {{v}}, would be ambiguous as an overload of
  // Values() (it can also list-initialize a lone Expr); Rows() sidesteps
  // that entirely.
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(
      InsertInto(tbl).Columns({Expr::FromRaw("a")}).Rows({{1}, {2}}).ToString(),
      "INSERT INTO foo (a) VALUES (1), (2)");
}

TEST(Insert, RowsAlreadySetThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(Ignore(InsertInto(tbl)
                          .Columns({Expr::FromRaw("a")})
                          .Rows({{1}, {2}})
                          .Rows({{3}})),
               std::logic_error);
}

TEST(Insert, ValuesAlreadySetThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(Ignore(InsertInto(tbl)
                          .Columns({Expr::FromRaw("a")})
                          .Values({1})
                          .Values({2})),
               std::logic_error);
}

TEST(Insert, RowsArityMismatchThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(Ignore(InsertInto(tbl)
                          .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                          .Rows({{1, 2}, {3}})),
               std::logic_error);
}

TEST(Insert, Returning) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a")})
                .Values({1})
                .Returning(Expr::FromRaw("id"))
                .ToString(),
            "INSERT INTO foo (a) VALUES (1) RETURNING id");
  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a")})
                .Values({1})
                .Returning({Expr::FromRaw("id"), Expr::FromRaw("a")})
                .ToString(),
            "INSERT INTO foo (a) VALUES (1) RETURNING id, a");
}

TEST(Insert, ReturningAlreadySetThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(Ignore(InsertInto(tbl)
                          .Columns({Expr::FromRaw("a")})
                          .Values({1})
                          .Returning(Expr::FromRaw("id"))
                          .Returning(Expr::FromRaw("a"))),
               std::logic_error);
}

TEST(Insert, OnConflictDoNothing) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a")})
                .Values({1})
                .OnConflictDoNothing()
                .ToString(),
            "INSERT INTO foo (a) VALUES (1) ON CONFLICT DO NOTHING");
  EXPECT_EQ(InsertInto(tbl)
                .Columns({Expr::FromRaw("a")})
                .Values({1})
                .OnConflictDoNothing({Expr::FromRaw("a")})
                .ToString(),
            "INSERT INTO foo (a) VALUES (1) ON CONFLICT (a) DO NOTHING");
}

TEST(Insert, OnConflictDoUpdate) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(
      InsertInto(tbl)
          .Columns({Expr::FromRaw("id"), Expr::FromRaw("count")})
          .Values({1, 1})
          .OnConflictDoUpdate(
              {Expr::FromRaw("id")},
              {{Expr::FromRaw("count"), Expr::FromRaw("EXCLUDED.count")}})
          .ToString(),
      "INSERT INTO foo (id, count) VALUES (1, 1) ON CONFLICT (id) DO UPDATE "
      "SET count = EXCLUDED.count");
}

TEST(Insert, OnConflictDoUpdateEmptyAssignmentsThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(Ignore(InsertInto(tbl)
                          .Columns({Expr::FromRaw("a")})
                          .Values({1})
                          .OnConflictDoUpdate({Expr::FromRaw("a")}, {})),
               std::invalid_argument);
}

TEST(Insert, ColumnsAlreadySetThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(Ignore(InsertInto(tbl)
                          .Columns({Expr::FromRaw("a")})
                          .Columns({Expr::FromRaw("b")})),
               std::logic_error);
}

TEST(Insert, ArityMismatchThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(Ignore(InsertInto(tbl)
                          .Columns({Expr::FromRaw("a"), Expr::FromRaw("b")})
                          .Values({1})),
               std::logic_error);
  EXPECT_THROW(Ignore(InsertInto(tbl).Values({1}).Columns(
                   {Expr::FromRaw("a"), Expr::FromRaw("b")})),
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

TEST(Update, From) {
  Table tbl = Table::FromRaw("foo");
  Table other = Table::FromRaw("bar");

  EXPECT_EQ(Update(tbl)
                .Set(Expr::FromRaw("age"), Expr::FromRaw("bar.age"))
                .From(other)
                .Where(Condition::FromRaw("foo.id = bar.id"))
                .ToString(),
            "UPDATE foo SET age = bar.age FROM bar WHERE foo.id = bar.id");
}

TEST(Update, FromJoin) {
  Table tbl = Table::FromRaw("foo");
  Table a = Table::FromRaw("a");
  Table b = Table::FromRaw("b");

  EXPECT_EQ(Update(tbl)
                .Set(Expr::FromRaw("x"), 1)
                .From(Join(a, b, Inner()).On(Condition::FromRaw("a.id = b.id")))
                .ToString(),
            "UPDATE foo SET x = 1 FROM a INNER JOIN b ON a.id = b.id");
}

TEST(Update, FromAliasedSubquery) {
  Table tbl = Table::FromRaw("foo");
  Table bar = Table::FromRaw("bar");

  EXPECT_EQ(Update(tbl)
                .Set(Expr::FromRaw("x"), 1)
                .From(From(bar).Select(Expr::FromRaw("*")).As("s"))
                .ToString(),
            "UPDATE foo SET x = 1 FROM (SELECT * FROM bar) AS s");
}

TEST(Update, FromUnaliasedSubqueryThrows) {
  Table tbl = Table::FromRaw("foo");
  Table bar = Table::FromRaw("bar");

  EXPECT_THROW(Ignore(Update(tbl)
                          .Set(Expr::FromRaw("x"), 1)
                          .From(From(bar).Select(Expr::FromRaw("*")))),
               std::logic_error);
}

TEST(Update, FromAlreadySetThrows) {
  Table tbl = Table::FromRaw("foo");
  Table a = Table::FromRaw("a");
  Table b = Table::FromRaw("b");

  EXPECT_THROW(Ignore(Update(tbl).Set(Expr::FromRaw("x"), 1).From(a).From(b)),
               std::logic_error);
}

TEST(Update, WhereAlreadySetThrows) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_THROW(Ignore(Update(tbl)
                          .Set(Expr::FromRaw("x"), 1)
                          .Where(Condition::FromRaw("a = 1"))
                          .Where(Condition::FromRaw("b = 2"))),
               std::logic_error);
}

TEST(Update, MissingSetThrows) {
  Table tbl = Table::FromRaw("foo");
  EXPECT_THROW(Update(tbl).Where(Condition::FromRaw("a = 1")).ToString(),
               std::logic_error);
}

TEST(Update, Returning) {
  Table tbl = Table::FromRaw("foo");
  Column age{"age", "BIGINT"};

  EXPECT_EQ(Update(tbl).Set(age, 42).Returning(Expr::FromRaw("id")).ToString(),
            "UPDATE foo SET age = 42 RETURNING id");
  EXPECT_EQ(Update(tbl)
                .Set(age, 42)
                .Where(age == 1)
                .Returning({Expr::FromRaw("id"), age.As("old_age")})
                .ToString(),
            "UPDATE foo SET age = 42 WHERE age = 1 RETURNING id, age AS "
            "old_age");
}

TEST(Update, ReturningAlreadySetThrows) {
  Table tbl = Table::FromRaw("foo");
  Column age{"age", "BIGINT"};

  EXPECT_THROW(Ignore(Update(tbl)
                          .Set(age, 42)
                          .Returning(Expr::FromRaw("id"))
                          .Returning(Expr::FromRaw("age"))),
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

TEST(Select, ColumnAlias) {
  Table tbl = Table::FromRaw("foo");
  Column age{"age", "BIGINT"};

  EXPECT_EQ(From(tbl)
                .Select({age.As("years"), (age.ToExpr() + 1).As("next_year")})
                .ToString(),
            "SELECT age AS years, age + 1 AS next_year FROM foo");
  EXPECT_EQ(From(tbl).Select(Expr::CountAll().As("n")).ToString(),
            "SELECT COUNT(*) AS n FROM foo");
}

TEST(Select, AliasInvalidNameThrows) {
  Column age{"age", "BIGINT"};

  EXPECT_THROW(Ignore(age.As("not an identifier")), std::invalid_argument);
  EXPECT_THROW(Ignore(age.As("")), std::invalid_argument);
  // A column alias, unlike a CTE or table name, cannot be qualified.
  EXPECT_THROW(Ignore(age.As("a.b")), std::invalid_argument);
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

TEST(Column, ToExpr) {
  Column age{"age", "BIGINT"};

  EXPECT_EQ(age.ToExpr().IsNull().ToString(), "age IS NULL");
  EXPECT_EQ(age.ToExpr().Between(0, 18).ToString(), "age BETWEEN 0 AND 18");
  EXPECT_EQ((age.ToExpr() + 1).ToString(), "age + 1");
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

  EXPECT_EQ(alias.Dot("name").ToString(), "bar.name");
  EXPECT_EQ(alias.Dot(name).ToString(), "bar.name");
}

TEST(TableAlias, DotUsableDirectlyAsExprNoFromRawNeeded) {
  Table users = Table::FromRaw("users");
  TableAlias lhs = TableAlias::From("lhs");
  TableAlias rhs = TableAlias::From("rhs");
  Column name{"name", "TEXT"};

  EXPECT_EQ(From(Join(users.As("lhs"), users.As("rhs"), Inner())
                     .On(lhs.Dot(name) == rhs.Dot(name)))
                .Select(lhs.Dot(name).As("name"))
                .ToString(),
            "SELECT lhs.name AS name FROM users AS lhs INNER JOIN "
            "users AS rhs ON lhs.name = rhs.name");
}

TEST(TableAlias, InvalidNameThrows) {
  EXPECT_THROW(Ignore(TableAlias::From("not an identifier")),
               std::invalid_argument);
  EXPECT_THROW(Ignore(TableAlias::From("")), std::invalid_argument);
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

TEST(Column, ImplicitlyConvertsToExpr) {
  // Column::operator Expr() must stay non-explicit: a Column has to be
  // usable wherever a `const Expr &` parameter is expected, with no cast.
  Column age{"age", "BIGINT"};
  EXPECT_EQ(Expr::FromRaw("x").Between(age, 30).ToString(),
            "x BETWEEN age AND 30");
}

TEST(Expr, LiteralInjectionAttempt) {
  // Even a hostile value can't break out of the quoted literal.
  EXPECT_EQ(Expr::FromRaw("x")
                .In({Expr::Literal("'; DROP TABLE users; --")})
                .ToString(),
            "x IN ('''; DROP TABLE users; --')");
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
