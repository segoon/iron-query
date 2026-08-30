#include "test_helpers.hpp"

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

TEST(WithFormatted, Single) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(With("cte", From(tbl).Select(Expr::FromRaw("a")))
                .Main(From(Table::FromRaw("cte")).Select(Expr::FromRaw("*")))
                .ToStringFormatted(),
            "WITH\n"
            "    cte AS (\n"
            "        SELECT\n"
            "            a\n"
            "        FROM\n"
            "            foo\n"
            "    )\n"
            "SELECT\n"
            "    *\n"
            "FROM\n"
            "    cte");
}

TEST(WithFormatted, Chained) {
  Table tbl = Table::FromRaw("foo");

  EXPECT_EQ(With("a", From(tbl).Select(Expr::FromRaw("x")))
                .With("b", From(tbl).Select(Expr::FromRaw("y")))
                .Main(From(Table::FromRaw("a")).Select(Expr::FromRaw("*")))
                .ToStringFormatted(),
            "WITH\n"
            "    a AS (\n"
            "        SELECT\n"
            "            x\n"
            "        FROM\n"
            "            foo\n"
            "    ),\n"
            "    b AS (\n"
            "        SELECT\n"
            "            y\n"
            "        FROM\n"
            "            foo\n"
            "    )\n"
            "SELECT\n"
            "    *\n"
            "FROM\n"
            "    a");
}
