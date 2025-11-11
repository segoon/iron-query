#include <gtest/gtest.h>

#include <builder.hpp>

using namespace sql_builder;

TEST(Select, From) {
    Table tbl("test");

    EXPECT_EQ(From(tbl).Select("*").ToString(), "SELECT * FROM test");
}

TEST(Select, SelectSmthFrom) {
    Table tbl("test");

    EXPECT_EQ(From(tbl).Select("a, b").ToString(), "SELECT a, b FROM test");
}

TEST(Select, Multi) {
    Table tbl("test");

    EXPECT_EQ(From(tbl).Select({"a", "b"}).ToString(), "SELECT a, b FROM test");
}

TEST(Select, Where) {
    Table tbl("test");

    EXPECT_EQ(From(tbl).Select("*").Where("a = b").ToString(), "SELECT * FROM test WHERE a = b");
}

TEST(Select, OrderBy) {
    Table tbl("test");

    EXPECT_EQ(From(tbl).Select("*").OrderBy("name").ToString(), "SELECT * FROM test ORDER BY name");
}

TEST(Select, Full) {
    Table tbl("test");

    EXPECT_EQ(
        From(tbl).Select("a, b").Where("a = b").OrderBy("name").ToString(),
        "SELECT a, b FROM test WHERE a = b ORDER BY name"
    );
}

TEST(Delete, From) {
    Table tbl("test");

    EXPECT_EQ(DeleteFrom(tbl).ToString(), "DELETE FROM test");
}

TEST(Delete, Where) {
    Table tbl("test");

    EXPECT_EQ(DeleteFrom(tbl).Where("a = 1").ToString(), "DELETE FROM test WHERE a = 1");
}

TEST(Join, Inner) {
    Table tbl1("foo");
    Table tbl2("bar");

    EXPECT_EQ(Join(tbl1, tbl2, Inner()).ToString(), "foo INNER JOIN bar");
}

TEST(Join, Cross) {
    Table tbl1("foo");
    Table tbl2("bar");

    EXPECT_EQ(Join(tbl1, tbl2, Cross()).ToString(), "foo CROSS JOIN bar");
}

TEST(Join, On) {
    Table tbl1("foo");
    Table tbl2("bar");

    EXPECT_EQ(Join(tbl1, tbl2, Cross()).On("foo.a = bar.b").ToString(), "foo CROSS JOIN bar ON foo.a = bar.b");
}

TEST(Join, SelectSelect) {
    Table tbl("foo");

    EXPECT_EQ(
        Join(From(tbl).Select("a"), From(tbl).Select("b"), Cross()).On("a = b").ToString(),
        "(SELECT a FROM foo) CROSS JOIN (SELECT b FROM foo) ON a = b"
    );
}

TEST(Column, Select) {
    Table tbl("foo");
    Column name{"name", "TEXT"};
    Column age{"age", "BIGINT"};

    EXPECT_EQ(From(tbl).Select({name, age}).ToString(), "SELECT name, age FROM foo");
}

TEST(Column, As) {
    Table tbl("foo");
    Column name{"name", "TEXT"};
    Column age{"age", "BIGINT"};

    EXPECT_EQ(From(tbl.As("bar")).Select("bar.name").ToString(), "SELECT bar.name FROM (foo AS bar)");
}
