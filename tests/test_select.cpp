#include "test_helpers.hpp"

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
