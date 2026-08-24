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
Table users = Table::FromRaw("users");
TableAlias left = TableAlias::From("left");
TableAlias right = TableAlias::From("right");
Column name{"name", "TEXT"};
Column age{"age", "BIGINT"};

std::string query = Join(
    From(users.As("left")).Select(Expr::FromRaw("*")),
    From(users.As("right")).Select({name, age}),
    Inner()
).On(Expr::FromRaw(left.Dot("second_name")) == Expr::FromRaw(right.Dot(name)))
    .ToString();

query = DeleteFrom(users).Where(age < 10).ToString();
```


# A note on untrusted input

`Expr::FromRaw(std::string)` treats its argument as a **trusted, raw SQL fragment** written
by the developer — it is inserted into the query verbatim, unescaped. The same applies to
every other `FromRaw`/`*Raw`-named entry point in the API (`Table::FromRaw`,
`TableWithColumns::FromRaw`, `Condition::FromRaw`, `Expr::CastRaw`, `Collation::FromRaw`).
Never pass untrusted/user-controlled data to any of these, or you will have a SQL injection
vulnerability.

A few other entry points take identifier-like arguments (`Expr::Call`'s function name,
`With`'s CTE name, `TableAlias::From`'s alias) and validate them as a plain or
dot-qualified SQL identifier, throwing `std::invalid_argument` otherwise — these are safer
to feed with dynamic input than the raw fragments above, though `Expr::Ident` is still the
right choice for column/table names that need proper quoting.

`WHERE`/`HAVING`/`ON`/`WHEN` all take a `Condition`, not a plain `Expr`, so a non-boolean
expression (`Where(age)`, `Where(age + 1)`) won't compile — this is a syntax-role check,
not a value-type check (the library still doesn't know or care whether `age` is an int or
text column). A boolean-valued column or expression needs to be turned into a `Condition`
explicitly, e.g. `Where(is_active.IsTrue())`.

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
