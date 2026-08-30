#include "test_helpers.hpp"

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

TEST(Join, Using) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, Inner()).Using({Expr::Ident("id")}).ToString(),
            "foo INNER JOIN bar USING (\"id\")");
}

TEST(Join, UsingMultipleColumns) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_EQ(Join(tbl1, tbl2, Inner())
                .Using({Expr::Ident("id"), Expr::Ident("tenant")})
                .ToString(),
            "foo INNER JOIN bar USING (\"id\", \"tenant\")");
}

TEST(Join, NaturalWithUsingThrows) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_THROW(
      Ignore(Join(tbl1, tbl2, NaturalInner()).Using({Expr::Ident("id")})),
      std::logic_error);
}

TEST(Join, UsingAlreadySetThrows) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_THROW(Ignore(Join(tbl1, tbl2, Inner())
                          .Using({Expr::Ident("id")})
                          .Using({Expr::Ident("tenant")})),
               std::logic_error);
}

TEST(Join, UsingThenOnThrows) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_THROW(Ignore(Join(tbl1, tbl2, Inner())
                          .Using({Expr::Ident("id")})
                          .On(Condition::FromRaw("foo.a = bar.a"))),
               std::logic_error);
}

TEST(Join, OnThenUsingThrows) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_THROW(Ignore(Join(tbl1, tbl2, Inner())
                          .On(Condition::FromRaw("foo.a = bar.a"))
                          .Using({Expr::Ident("id")})),
               std::logic_error);
}

TEST(Join, UsingEmptyThrows) {
  Table tbl1 = Table::FromRaw("foo");
  Table tbl2 = Table::FromRaw("bar");

  EXPECT_THROW(Ignore(Join(tbl1, tbl2, Inner()).Using({})),
               std::invalid_argument);
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
