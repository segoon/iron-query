# SQL coverage

Status of IronQuery's SQL surface as of the P0 batch (§5 items 1-4).

Reference dialect: **PostgreSQL 17**
([SQL commands](https://www.postgresql.org/docs/current/sql-commands.html),
[expressions](https://www.postgresql.org/docs/current/sql-expressions.html),
[SELECT](https://www.postgresql.org/docs/current/sql-select.html)), which is what
the precedence table in `OperatorPrecedence` and the `$N` placeholders (`_1`..`_10`)
already assume.

**Rarity rating**: how often a typical application developer needs the feature when
building *dynamic* queries. `0` = exotic, `10` = in almost every codebase.
Anything the developer can still express through `Expr::FromRaw`/`Condition::FromRaw`
is "unsupported" here in the sense that it has no safe, typo-proof API — which is the
whole point of the product (see `docs/VISION.md`).

---

## 1. Statements

### Supported

| Statement | Notes |
|---|---|
| `SELECT` | `SelectExpr`: select list (with `AS` aliases), `FROM` (single item), `WHERE`, `GROUP BY`, `HAVING`, `ORDER BY` (with `ASC`/`DESC`), `LIMIT`, `OFFSET` |
| `INSERT INTO ... VALUES (...)` | `InsertInto`: single row, explicit column list |
| `UPDATE ... SET ... WHERE` | `Update` |
| `DELETE FROM ... WHERE` | `DeleteFrom` |
| `JOIN` | `Join` + `Inner`/`Cross`/`LeftOuter`/`RightOuter`/`FullOuter`, optional `ON`. Usable as a `FROM` source: `From(Join(a, b, Inner()).On(cond))` |
| `UNION` / `UNION ALL` / `INTERSECT` / `EXCEPT` | `SetOp` |
| `WITH ... AS (...)` | `With(...)...Main(...)`, N non-recursive CTEs |
| Subqueries | `VirtualTable::operator Expr` (scalar subquery), `Expr::Exists`/`NotExists`, CTE bodies |
| `SELECT DISTINCT` | `SelectExpr::Distinct()` |
| `ORDER BY ... NULLS FIRST/LAST` | `OrderByTerm`'s third constructor argument, `NullsOrder` |
| `INSERT` with a bind-parameter value | `Values()` already takes `Expr`, and `_1..._10` are `Expr`, so `Values({_1, _2})` renders `VALUES ($1, $2)` with no extra API |

### Unsupported

| Statement / clause | Rarity | Notes |
|---|---|---|
| `DISTINCT ON` | 5 | No API; plain `DISTINCT` is covered by `SelectExpr::Distinct()`. |
| Multi-row `INSERT ... VALUES (a),(b)` | 8 | `Values()` can only be called for one row (second call appends into the same tuple). |
| `INSERT ... RETURNING` / `UPDATE`/`DELETE ... RETURNING` | 8 | PG-specific but ubiquitous there. |
| `INSERT ... ON CONFLICT DO NOTHING/UPDATE` (upsert) | 8 | |
| Multiple `FROM` items (`FROM a, b`) | 7 | Only one item; a join covers most of the need. |
| `INSERT INTO ... SELECT` | 7 | `Values()` only takes an `Expr` list. |
| `LIMIT`/`OFFSET` by bind parameter | 7 | `Limit(int)`/`Offset(int)` take `int`, so `LIMIT $1` is impossible. |
| `UPDATE ... FROM`, `DELETE ... USING` | 6 | |
| `JOIN ... USING (cols)` / `NATURAL JOIN` | 5 | Only `ON`. |
| `ORDER BY`/`LIMIT` applied to a `UNION` result | 5 | `SetOp` has no clauses of its own. |
| `WITH RECURSIVE` | 4 | |
| `SELECT` with no `FROM` (`SELECT now()`) | 4 | `EnsureValid()` requires `FROM`. |
| `FOR UPDATE` / `FOR SHARE` / `SKIP LOCKED` | 4 | Job-queue patterns. |
| `LATERAL` subqueries | 3 | |
| CTE `name(col, ...)` column lists; `MATERIALIZED` hints | 2 | |
| `GROUPING SETS` / `ROLLUP` / `CUBE` | 2 | |
| `VALUES` as a standalone table source | 2 | |
| `MERGE` | 1 | |
| DDL (`CREATE`/`ALTER`/`DROP TABLE`, indexes) | 3 | Deliberately out of scope? Worth stating explicitly. Migration tooling usually owns this. |
| Transaction control (`BEGIN`/`COMMIT`), `EXPLAIN`, `COPY`, `GRANT`, cursors | 1 | Out of scope; a query *builder* is the wrong layer. |

---

## 2. Expressions

### Supported

| Feature | API |
|---|---|
| Escaped string literal | `Expr::Literal` |
| Escaped identifier | `Expr::Ident` |
| Bind placeholders `$1..$10` | `_1` .. `_10` |
| Raw fragment (trusted) | `Expr::FromRaw`, optionally with an explicit `OperatorPrecedence` |
| Arithmetic `+ - * / % ^` | operators, with correct auto-parenthesization |
| Comparison `< <= > >= = !=` | operators → `Condition` |
| `AND` / `OR` / `NOT` | `Condition::operator&& \|\| !` |
| `BETWEEN` / `NOT BETWEEN` | `Expr::Between`/`NotBetween` |
| `LIKE` / `NOT LIKE` | `Expr::Like`/`NotLike` |
| `ILIKE` / `NOT ILIKE` | `Expr::ILike`/`NotILike` |
| `SIMILAR TO` / `NOT SIMILAR TO` | `Expr::SimilarTo`/`NotSimilarTo` |
| `IS [NOT] DISTINCT FROM` | `Expr::IsDistinctFrom`/`IsNotDistinctFrom` |
| `IN` / `NOT IN` | `Expr::In`/`NotIn`, over a value list or a subquery |
| `= ANY` / `<> ALL` | `Expr::EqAny`/`NeAll`, over an array expression or a subquery |
| `NULL` / `TRUE` / `FALSE` literals | `Expr::Null`, `Expr::Bool` |
| Numeric literals | `Expr(T)` for any integer width/signedness and for `float`/`double` (round-trip-exact) |
| Column alias | `Expr::As`/`Column::As`, returning a `SelectItem` usable only in a select list |
| `IS [NOT] NULL`, `IS TRUE/FALSE` | `Expr::IsNull` etc. |
| `EXISTS` / `NOT EXISTS` | `Expr::Exists`/`NotExists` |
| Field access `a.b`, subscript `a[i]` | `Expr::Dot`, `operator[]` |
| `CAST(x AS t)` | `Expr::CastRaw` (type is raw) |
| `COLLATE` | `Expr::Collate` + `Collation` |
| Function call | `Expr::Call(name, {args})`, name validated as an identifier |
| Aggregates | `Count`, `CountAll`, `CountDistinct`, `Sum`, `Avg`, `Min`, `Max` |
| `CASE WHEN ... THEN ... ELSE ... END` | `Case()...When().Then().Else().End()` (searched form) |

### Unsupported

| Feature | Rarity | Notes |
|---|---|---|
| String concatenation `\|\|` | 8 | Collides with `operator\|\|` on `Condition`; needs a named `Concat`. |
| Unary minus, unary `NOT` on `Expr` | 8 | `-x` has no API. |
| `~` / `!~` regex | 6 | PG-specific, common in search filters. |
| Window functions (`OVER (PARTITION BY ... ORDER BY ...)`) | 6 | `row_number()`, `rank()`, running totals. |
| Aggregate modifiers: `FILTER (WHERE ...)`, `ORDER BY` inside aggregates | 5 | `COUNT(DISTINCT x)` is covered by `Expr::CountDistinct`. |
| More aggregates: `string_agg`, `array_agg`, `json_agg`, `bool_and/or` | 5 | `Expr::Call` covers them, but arg-count/typo safety is lost for none — this is arguably fine. |
| `COALESCE` / `NULLIF` / `GREATEST` / `LEAST` | 6 | Reachable via `Expr::Call`; deserve named helpers since they're keywords, not functions. |
| Simple `CASE expr WHEN v THEN ...` form | 4 | Only the searched form exists. |
| `x::type` shorthand and a typed (non-raw) type vocabulary | 5 | `CastRaw` is the only cast, and it's a raw hole. |
| Array constructors / operators (`ARRAY[...]`, `@>`, `&&`, `ANY`) | 4 | |
| Row constructors, `(a,b) IN (...)`, `IS NULL` on rows | 3 | |
| JSON/JSONB operators (`->`, `->>`, `#>`, `@>`) | 5 | Common in PG apps; all require `FromRaw` today. |
| Bitwise `& \| # << >>` | 3 | |
| `LIKE ... ESCAPE` | 2 | |
| `EXTRACT(field FROM x)`, `INTERVAL '...'`, date/time literals | 5 | |
| `AT TIME ZONE` | 3 | `kAt` precedence exists but no operator uses it. |
| Ordered-set/grouped aggregates (`percentile_cont ... WITHIN GROUP`) | 1 | |
| `GROUPING()`, `TABLESAMPLE`, `IS OF`, `OVERLAPS` | 0-1 | |

---

## 3. Schema / safety features

### Supported

- `Column{name, type, is_nullable}` with comparison operators and implicit `Expr` conversion.
- `TableWithColumns` + `SelectArgAll()` — the "no lost field after migration" story.
- `TableAlias::From()` with identifier validation; `TableAlias::Dot`.
- Trust boundary is explicit: every unescaped entry point is named `*Raw`.
- `Condition` vs `Expr` separation, so `Where(age + 1)` does not compile.
- `SelectItem` vs `Expr` separation, so `Where(x.As("y"))` does not compile either.
- `Expr(bool)`/`Expr(char)`/`Expr(const char*)` are deleted rather than silently
  rendered as numbers, so `Expr("text")` is a compile error instead of `TRUE`.
- FROM items are validated: an unaliased subquery in `FROM` throws at build time
  instead of failing on the server.
- Automatic parenthesization driven by the PG precedence table.

### Gaps

| Feature | Rarity | Notes |
|---|---|---|
| `Column` cannot produce a *qualified* `Expr` | **9** | `TableAlias::Dot` returns `std::string`, so every qualified reference in the README goes through `Expr::FromRaw(left.Dot(name))`. The library's flagship safety feature is bypassed in its own quick-start example. |
| `Column` has no `IsNull`/`Like`/`In`/`Between`/arithmetic | 8 | Only the six comparisons and `As` are duplicated onto `Column`; everything else needs an explicit `Expr(col)`. Duplication that should be solved by one conversion path, not by more overloads. |
| `is_nullable` is stored and never used | 7 | Could reject `col == NULL`, or warn on `!=` against a nullable column. |
| `Column.type` is stored and never used | 6 | Could power a typed `Cast`, or reject `text_col + int_col`. |
| No `TableWithColumns` → alias binding | 8 | Nothing ties an alias to a column set, so `alias.Dot(col)` can't verify `col` belongs to the table. |
| `Expr::Ident` doesn't reject embedded quotes' cousins | 3 | Escaping is correct (`""` doubling, NUL rejected), but the identifier-length limit (63 bytes in PG) is silently exceeded. |
| No `Column`/table generation from a schema | 8 | The "after migration" promise needs a codegen step or a macro; currently the developer hand-writes `Column{...}`. |
| No dialect abstraction | 6 | `$N` placeholders, `COLLATE`, `!=` vs `<>` are all hardcoded PG-ish. MySQL/SQLite would need `?` placeholders and backtick quoting. |
| No parameter binding container | 8 | `_1.._10` are just text; the values are the caller's problem, and nothing checks that `$3` was actually supplied. Also hard-capped at 10. |

---

## 4. Defects found while auditing — all fixed

Five behavioural defects turned up during this audit; all are fixed, each with a
regression test. Recorded here because two of them shaped the priorities in §5.

1. **`SelectExpr::Select(initializer_list)` appended instead of replacing.** Two calls
   silently concatenated. `OrderBy`/`GroupBy` assigned in both overloads, so it was a
   slip, not a design. Fixed by giving each comma-separated clause a single assignment
   site (`RenderAll` in `src/detail/render.hpp`), with the one-term overloads delegating
   to the many-term ones — the two can no longer disagree.
2. **`VirtualTable::As` wrapped the alias inside the parentheses**, emitting
   `(foo AS bar)`, `((SELECT …) AS s)` and `((a JOIN b) AS j)`. PostgreSQL's
   `table_ref` attaches the alias directly to the item and permits parentheses in
   exactly one case, `'(' joined_table ')' alias_clause`, so the first two were
   unparseable. Dropping the outer parens fixes all three, because `ToStringBracketed()`
   already brackets per type. `As()` now also validates its alias as an identifier,
   closing an unmarked raw-SQL hole.
3. **`Expr::operator[]` was tagged `kSymbol` instead of `kIndex`**, unlike every
   sibling operator. Harmless in output, but it made `a[b].c` render unbracketed.
4. **`InsertInto` did not check column/value arity** — `(a, b) VALUES (1)` built fine
   and failed at the server, exactly the typo class the product exists to prevent.
   Both setters now replace rather than accumulate, and cross-check counts eagerly.
5. **Member initialisation order disagreed with declaration order** in `Expr` and
   `Condition`. Fixed, and `-Wall -Wextra -Werror` is now on both build targets — which
   promptly surfaced **two more instances of the same defect, in `Join` and `SetOp`**,
   that the audit's manual read had missed. That is the argument for the flag.

**Caveat:** no PostgreSQL or `libpg_query` is available in this environment, so the
corrected strings were checked against the grammar by reading it, not by parsing them.
Defect #2 is precisely what a string-comparison-only suite cannot catch — a test was
asserting `FROM (foo AS bar)`, pinning the bug. See P0 item 6 in §5. The same caveat
applies to everything added for P0 items 1-4.

---

## 5. Proposed TODO, in priority order

Ordered by (rarity × how badly the gap forces users back into raw strings).

**P0 — the library is hard to use without these**

1. ~~Make `Join` a first-class `FROM` source.~~ Done. `From()`/`SelectExpr` now take a
   `const VirtualTable&` and render it through a new `ToStringAsFromItem()` virtual:
   `Table` returns its bare name, `Join` returns itself **unbracketed** (PostgreSQL's
   `table_ref` allows `'(' joined_table ')'` only when an alias follows), and everything
   else — every subquery — throws, since PostgreSQL requires an alias there. Use `As()`.
   **`DeleteFrom`/`Update` deliberately still take a `const Table&`:** their target is a
   plain `relation_expr`, never a join or subquery, so widening them would only let the
   builder emit SQL that cannot parse. No `SelectExpr::Join(...)` chain step either —
   `From(Join(a, b, Inner()).On(cond))` already composes, and a second builder type for
   it would earn nothing.
2. ~~`Expr::Null()`, `Expr::Bool()`, `Expr(double)`, `Expr(int64_t)`.~~ Done, via
   constrained template constructors (one for integers of any width and signedness, one
   for floating point) plus deleted `bool`/`char`/`const char*` overloads — plain
   overloads would have made `Expr(unsigned)` ambiguous and let `Expr("text")` decay to
   `TRUE`. Doubles render through `std::to_chars` (shortest round-trip); NaN/infinity
   throw, since SQL spells them as typed literals.
3. ~~`IN` with a value list and with a subquery.~~ Done, plus `EqAny`/`NeAll` for the
   bind-parameter form. The old `In(const Expr&)`/`NotIn(const Expr&)` are **removed**:
   they emitted `x IN a` unparenthesized, so the only way to use them was to concatenate
   `"(1, 2, 3)"` by hand.
4. ~~Column aliases in the select list.~~ Done. `Expr::As`/`Column::As` return a
   `SelectItem`, a new type that only `Select()` accepts, so `x AS y` cannot leak into a
   `WHERE` or a function argument.
5. ~~Fix defects #1 and #4.~~ Done — see §4.
6. **Validate generated SQL against a real parser.** Every test is still a string
   comparison, which is why defect #2 survived to be found by reading. Feeding each
   expected string through `postgres --check`/`libpg_query`/an ephemeral PG container in
   CI would turn the whole suite into a syntax oracle — the cheapest possible guard for
   a library whose entire value proposition is "your SQL parses". **Now the only open P0
   item, and the highest-value one**: the §4 fixes and everything added for items 1-4
   above are grammar-checked by hand, not by a parser. Deliberately deferred out of the
   P0 batch as CI/infrastructure work rather than API work.

**P1 — routine work that currently needs `FromRaw`**

6. ~~`SELECT DISTINCT`~~ Done, via `SelectExpr::Distinct()`. `DISTINCT ON` remains unsupported.
7. Multi-row `INSERT`, `INSERT ... SELECT`, `RETURNING`, `ON CONFLICT`.
8. `Limit`/`Offset` taking an `Expr` so bind parameters work. (`INSERT`'s `Values()` already
   accepts bind placeholders, since it takes plain `Expr` and `_1..._10` are `Expr`.)
9. ~~`ORDER BY ... NULLS FIRST/LAST`~~ Done, via `OrderByTerm`'s `NullsOrder` parameter.
10. ~~`COUNT(DISTINCT x)`~~ Done, via `Expr::CountDistinct`. `FILTER (WHERE ...)` and named
    `Coalesce`/`NullIf`/`Greatest`/`Least` remain open.
11. ~~`ILIKE` / `IS [NOT] DISTINCT FROM`~~ Done, via `Expr::ILike`/`NotILike`/`SimilarTo`/
    `NotSimilarTo`/`IsDistinctFrom`/`IsNotDistinctFrom`. Regex match (`~`/`!~`) remains open.
12. Unary minus and string concatenation (`Expr::Concat`, since `||` is taken).

**P2 — makes the schema-safety promise real**

13. **Qualified column references end to end**: `TableAlias::Dot` should return `Expr`,
    and `TableWithColumns` should hand out `Column`s already bound to their table/alias,
    so the README example needs no `FromRaw` at all. This subsumes the `Column` operator
    duplication — make `Column` convert to a fully-qualified `Expr` once instead of
    re-declaring operators on it.
14. Use `Column::is_nullable` and `Column::type` for something (reject `== NULL`, type a
    `Cast`), or drop them.
15. A schema→`TableWithColumns` codegen path, so "lost field after migration" is a
    compile error rather than a convention.

**P3 — breadth**

16. Window functions.
17. JSON operators, arrays, `EXTRACT`/`INTERVAL`.
18. `WITH RECURSIVE`, `LATERAL`, `FOR UPDATE`.
19. Dialect abstraction (placeholder style, identifier quoting) if non-PG targets matter.

**Explicitly out of scope** (worth writing into `docs/VISION.md`): DDL, transaction
control, `EXPLAIN`, `COPY`, permissions, and anything an ORM would do (mapping rows to
objects, identity maps, lazy loading).
