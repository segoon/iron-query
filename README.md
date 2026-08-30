# IronQuery

This is a SQL query builder for C++.
Note: this is NOT an ORM!

Licensed under the [Apache License 2.0](LICENSE).

# Do I need IronQuery?

Answer the following questions:
1) Are your queries structure defined in runtime (e.g. optional `LIMIT`/`WHERE` or dynamic `WHERE` clause)?
2) Do you want to keep yourself distant from SQL?
3) Is the query performance critical?

# Features

1) `*Raw`-named entry points (`Expr::FromRaw`, `Table::FromRaw`, ...) mark every
   place unescaped SQL can enter a query, so the untrusted-input surface can be
   audited by grepping for `Raw`;
2) `Condition`/`Expr`/`SelectItem` are distinct types, so a non-boolean
   expression in `WHERE`/`HAVING`/`ON` or a select-list-only alias used
   elsewhere fails to compile instead of building a wrong query;
3) Every clause-setting method (`WHERE`, `LIMIT`, `FROM`/`USING`, `SELECT`, ...)
   can be set at most once per builder, so a copy-paste bug like
   `DELETE ... USING ... USING ...` throws instead of silently overwriting
   the first clause;
4) No template-metaprogramming EDSL: queries are built with ordinary
   OOP, not deeply nested templates, so compile errors stay readable;
5) `TableWithColumns::As()`/`Dot()` binds an alias to its declared column
   list, so a qualified reference to a column that doesn't belong to the
   table throws at build time instead of failing on the server.

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
// "left"/"right" would not do: they are reserved words in PostgreSQL.
TableAlias lhs = TableAlias::From("lhs");
TableAlias rhs = TableAlias::From("rhs");
Column name{"name", "TEXT"};
Column age{"age", "BIGINT"};

std::string query = From(
    Join(users.As("lhs"), users.As("rhs"), Inner())
        .On(lhs.Dot("second_name") == rhs.Dot(name)))
    .Select({lhs.Dot(name).As("name"), age})
    .Where(Expr(age).Between(18, 65) && Expr(age).NotIn({30, 40}))
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
`With`'s CTE name, `TableAlias::From`'s and `VirtualTable::As`'s alias) and validate them as a plain or
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


# Build dependencies

The library itself is header-only and has no dependencies beyond a C++17 compiler.
Building/testing/linting the project needs, depending on the `make` target used:

* `test` — CMake (>= 3.16), a C++17 compiler (GCC or Clang), GoogleTest (`libgtest-dev`)
* `format`/`format-check` — `clang-format`
* `tidy` — `clang-tidy`
* `sanitize` — a C++17 compiler with `-fsanitize=address,undefined` support
* `docs` — `doxygen` (optionally `graphviz`/`dot`, for inheritance/collaboration graphs)


# Documentation

Run `make docs` to generate the API reference from the doxygen comments in `include/`;
open `build/docs/html/index.html` to view it.
