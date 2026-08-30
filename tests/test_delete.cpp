#include "test_helpers.hpp"

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
