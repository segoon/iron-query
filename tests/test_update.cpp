#include "test_helpers.hpp"

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
