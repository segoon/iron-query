#include "test_helpers.hpp"

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
