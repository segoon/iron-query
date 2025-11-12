# SQL query builder++

This is a header-only SQL query builder for C++.
Note: this is NOT an ORM!


# Architectual decisions

1) This is a plain C++ -> SQL mapping builder;
2) This is NOT an ORM;
3) The main idea behind this is a possibility to avoid typical
   syntax mistakes/typos in SQL query typing;
4) The builder DOES NOT check for identifier definition,
   expression data types, member names, etc.
5) The builder DOES NOT check for missed required clause.

The benefits of using the builder:
1) Dynamically build complex WHERE clauses with something
   better than raw strings concatenation;
2) Automatically use full set of table column names after schema migration;
3) Ignore clauses order (e.g. WHERE/ORDER BY/LIMIT).


# Cost of dynamic builder

Query builder can look cool, but it is not free.
Compared to plain SQL query as a string literal,
the builder stores temporary data in struct fields,
concatenates substrings, etc.
If you have to build queries in runtime, it's OK.
But if you just want to run a static SQL query,
you probably don't needd SQL query builder.


# Quick start

The builder library is header-only, so you just include the header and use it:

```cpp
#include <sql_builder_pp.hpp>

using namespace sql_builder_pp;

...
Table users("users");
TableAlias left("left");
TableAlias right("right");
Column name("name", "TEXT");
Column name("age", "BIGINT");

std::string query = Join(
    From(users.As(left)).Select("*"),
    From(users.As(right)).Select({name, age}),
    Inner()
).On(left.Dot("second_name") == right.Dot(name)).ToString();

query = DeleteFrom(table).Where(age < 10).ToString();
```
