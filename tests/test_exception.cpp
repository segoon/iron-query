#include "test_helpers.hpp"
#include <limits>

TEST(Exception, LogicErrorIsAStdLogicError) {
  EXPECT_THROW(Ignore(From(Table::FromRaw("t")).ToString()), std::logic_error);
  try {
    Ignore(From(Table::FromRaw("t")).ToString());
    FAIL() << "expected LogicError";
  } catch (const LogicError &e) {
    EXPECT_NE(std::string(e.what()).find("iron_query: "), std::string::npos);
  }
}

TEST(Exception, InvalidIdentifierIsAnInvalidArgument) {
  EXPECT_THROW(Ignore(TableAlias::From("not an identifier")),
               std::invalid_argument);
  try {
    Ignore(TableAlias::From("not an identifier"));
    FAIL() << "expected InvalidIdentifier";
  } catch (const InvalidIdentifier &e) {
    EXPECT_NE(std::string(e.what()).find("iron_query: "), std::string::npos);
  }
}

TEST(Exception, InvalidOperatorIsAnInvalidArgument) {
  Expr x = Expr::FromRaw("x");
  EXPECT_THROW(Ignore(x.BinaryOp("not-an-operator", x)), std::invalid_argument);
  EXPECT_THROW(Ignore(x.BinaryOp("not-an-operator", x)), InvalidOperator);
}

TEST(Exception, InvalidLiteralIncludesTheValue) {
  try {
    Ignore(Expr(std::numeric_limits<double>::infinity()));
    FAIL() << "expected InvalidLiteral";
  } catch (const InvalidLiteral &e) {
    EXPECT_NE(std::string(e.what()).find("inf"), std::string::npos);
  }
}

TEST(Exception, UnknownColumnIncludesColumnAndTable) {
  TableWithColumns tbl =
      TableWithColumns::FromRaw("t", {Column{"a", "TEXT"}}).As("alias");
  try {
    Ignore(tbl.Dot("b"));
    FAIL() << "expected UnknownColumn";
  } catch (const UnknownColumn &e) {
    std::string what = e.what();
    EXPECT_NE(what.find('b'), std::string::npos);
    EXPECT_NE(what.find("alias"), std::string::npos);
  }
}
