# IronQuery

This is a SQL query builder for C++.
Note: this is NOT an ORM!

Licensed under the [Apache License 2.0](LICENSE).


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

Include the header and link against the `iron-query` library:

```cpp
#include <iron_query/iron_query.hpp>

using namespace iron_query;

...
Table users("users");
TableAlias left("left");
TableAlias right("right");
Column name("name", "TEXT");
Column age("age", "BIGINT");

std::string query = Join(
    From(users.As(left)).Select("*"),
    From(users.As(right)).Select({name, age}),
    Inner()
).On(left.Dot("second_name") == right.Dot(name)).ToString();

query = DeleteFrom(table).Where(age < 10).ToString();
```


# A note on untrusted input

`Expr(std::string)` / `Expr(const char*)` treat their argument as a **trusted, raw SQL
fragment** written by the developer — it is inserted into the query verbatim, unescaped.
Never pass untrusted/user-controlled data to `Expr(string)` directly, or you will have a SQL
injection vulnerability.

If you need to embed a value or an identifier that isn't a fixed string literal in your code
(a user-supplied search term, a dynamically chosen sort column, etc.), use the escaping
factories instead:

```cpp
// A value: properly single-quoted and escaped.
Expr::Literal(user_supplied_value)

// An identifier (table/column name): properly double-quoted and escaped.
Expr::Ident(dynamically_sourced_column_name)
```

Bind-parameter placeholders (`_1`..`_10`) remain the preferred way to pass values when your
SQL driver supports parameterized queries — prefer them over `Expr::Literal` whenever
possible.
