#include "test_helpers.hpp"

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
