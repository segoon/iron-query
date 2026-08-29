# SQL coverage

Status of IronQuery's SQL surface, kept in sync as features land (see §5 for
remaining work).

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
| `INSERT INTO ... VALUES (...)` | `InsertInto`: explicit column list, single row via `Values()` or multiple rows via `Rows()` |
| `INSERT ... RETURNING` / `UPDATE`/`DELETE ... RETURNING` | `InsertInto`/`Update`/`DeleteFrom::Returning()` |
| `INSERT ... ON CONFLICT DO NOTHING/UPDATE` (upsert) | `InsertInto::OnConflictDoNothing`/`OnConflictDoUpdate` |
| `UPDATE ... SET ... WHERE` | `Update` |
| `UPDATE ... FROM` | `Update::From`, taking a table, join, or aliased subquery like `SelectExpr`'s `From` |
| `DELETE FROM ... WHERE` | `DeleteFrom` |
| `DELETE ... USING` | `DeleteFrom::Using`, taking a table, join, or aliased subquery like `SelectExpr`'s `From` |
| `JOIN` | `Join` + `Inner`/`Cross`/`LeftOuter`/`RightOuter`/`FullOuter`, optional `ON`. Usable as a `FROM` source: `From(Join(a, b, Inner()).On(cond))` |
| `NATURAL JOIN` | `Join` + `NaturalInner`/`NaturalLeftOuter`/`NaturalRightOuter`/`NaturalFullOuter` (no `ON`; no `NaturalCross`, matching PostgreSQL's grammar) |
| `UNION` / `UNION ALL` / `INTERSECT` / `EXCEPT` | `SetOp` |
| `WITH ... AS (...)` | `With(...)...Main(...)`, N non-recursive CTEs |
| Subqueries | `VirtualTable::operator Expr` (scalar subquery), `Expr::Exists`/`NotExists`, CTE bodies |
| `SELECT DISTINCT` | `SelectExpr::Distinct()` |
| `SELECT DISTINCT ON (...)` | `SelectExpr::DistinctOn()`, mutually exclusive with `Distinct()` |
| `ORDER BY ... NULLS FIRST/LAST` | `OrderByTerm`'s third constructor argument, `NullsOrder` |
| `INSERT` with a bind-parameter value | `Values()` already takes `Expr`, and `_1..._10` are `Expr`, so `Values({_1, _2})` renders `VALUES ($1, $2)` with no extra API |

### Unsupported

| Statement / clause | Rarity | Notes |
|---|---|---|
| Multiple `FROM` items (`FROM a, b`) | 7 | Only one item; a join covers most of the need. |
| `INSERT INTO ... SELECT` | 7 | `Values()` only takes an `Expr` list. |
| `LIMIT`/`OFFSET` by bind parameter | 7 | `Limit(int)`/`Offset(int)` take `int`, so `LIMIT $1` is impossible. |
| `JOIN ... USING (cols)` | 5 | Only `ON`. |
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
| `COALESCE` / `NULLIF` / `GREATEST` / `LEAST` | `Expr::Coalesce`/`NullIf`/`Greatest`/`Least` |
| `CASE WHEN ... THEN ... ELSE ... END` | `Case()...When().Then().Else().End()` (searched form) |
| String concatenation `\|\|` | `Expr::Concat` (named rather than `operator\|\|`, which is `Condition`'s logical OR) |
| Unary minus | `Expr::operator-` |
| Unary `NOT` on `Expr` | `Expr::operator!`, distinct from `Condition::operator!` |

### Unsupported

| Feature | Rarity | Notes |
|---|---|---|
| `~` / `!~` regex | 6 | PG-specific, common in search filters. |
| Window functions (`OVER (PARTITION BY ... ORDER BY ...)`) | 6 | `row_number()`, `rank()`, running totals. |
| Aggregate modifiers: `FILTER (WHERE ...)`, `ORDER BY` inside aggregates | 5 | `COUNT(DISTINCT x)` is covered by `Expr::CountDistinct`. |
| More aggregates: `string_agg`, `array_agg`, `json_agg`, `bool_and/or` | 5 | `Expr::Call` covers them, but arg-count/typo safety is lost for none — this is arguably fine. |
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

- `Column{name, type, is_nullable}` with comparison operators, implicit `Expr`
  conversion, and an explicit `Column::ToExpr()` for reaching the rest of
  `Expr`'s API (`IsNull`, `Like`, `In`, `Between`, arithmetic, ...) without
  growing `Column`'s own overload set.
- `TableWithColumns` + `SelectArgAll()` — the "no lost field after migration" story.
- `TableAlias::From()` with identifier validation; `TableAlias::Dot` returns a
  ready-to-use, correctly-precedenced `Expr` (`alias.column`), so no
  `Expr::FromRaw` wrapping is needed to qualify a column reference.
- Trust boundary is explicit: every unescaped entry point is named `*Raw`.
- `Condition` vs `Expr` separation, so `Where(age + 1)` does not compile.
- `SelectItem` vs `Expr` separation, so `Where(x.As("y"))` does not compile either.
- `Expr(bool)`/`Expr(char)`/`Expr(const char*)` are deleted rather than silently
  rendered as numbers, so `Expr("text")` is a compile error instead of `TRUE`.
- FROM items are validated: an unaliased subquery in `FROM` throws at build time
  instead of failing on the server.
- Automatic parenthesization driven by the PG precedence table.
- Singular clauses (`WHERE`, `HAVING`, `LIMIT`, `OFFSET`, `FROM`/`USING` on
  `UPDATE`/`DELETE`, `ON` on `JOIN`) can be set at most once per builder; a
  second call throws instead of silently discarding the first — unlike the
  list clauses (`SELECT`, `GROUP BY`, `ORDER BY`, `RETURNING`), where a
  second call intentionally replaces the list.

### Gaps

| Feature | Rarity | Notes |
|---|---|---|
| `is_nullable` is stored and never used | 7 | Could reject `col == NULL`, or warn on `!=` against a nullable column. |
| `Column.type` is stored and never used | 6 | Could power a typed `Cast`, or reject `text_col + int_col`. |
| No `TableWithColumns` → alias binding | 8 | Nothing ties an alias to a column set, so `alias.Dot(col)` can't verify `col` belongs to the table. |
| `Expr::Ident` doesn't reject embedded quotes' cousins | 3 | Escaping is correct (`""` doubling, NUL rejected), but the identifier-length limit (63 bytes in PG) is silently exceeded. |
| No `Column`/table generation from a schema | 8 | The "after migration" promise needs a codegen step or a macro; currently the developer hand-writes `Column{...}`. |
| No dialect abstraction | 6 | `$N` placeholders, `COLLATE`, `!=` vs `<>` are all hardcoded PG-ish. MySQL/SQLite would need `?` placeholders and backtick quoting. |
| No parameter binding container | 8 | `_1.._10` are just text; the values are the caller's problem, and nothing checks that `$3` was actually supplied. Also hard-capped at 10. |

---

## 5. Proposed TODO, in priority order

Ordered by (rarity × how badly the gap forces users back into raw strings).

**P1 — routine work that currently needs `FromRaw`**

2. `INSERT INTO ... SELECT`.
3. `Limit`/`Offset` taking an `Expr` so bind parameters work.
4. `FILTER (WHERE ...)`.
5. Regex match (`~`/`!~`).

**P2 — makes the schema-safety promise real**

7. **`TableWithColumns` → alias binding**: hand out `Column`s already bound to their
   table/alias, so a qualified reference needs neither `FromRaw` nor a separate
   `TableAlias::Dot` call, and `alias.Dot(col)` can verify `col` actually belongs to the
   table. (`TableAlias::Dot` already returns `Expr` — see §3 Supported.) This subsumes the
   `Column` operator duplication — make `Column` convert to a fully-qualified `Expr` once
   instead of re-declaring operators on it.
8. Use `Column::is_nullable` and `Column::type` for something (reject `== NULL`, type a
   `Cast`), or drop them.
9. A schema→`TableWithColumns` codegen path, so "lost field after migration" is a
   compile error rather than a convention.

**P3 — breadth**

10. Window functions.
11. JSON operators, arrays, `EXTRACT`/`INTERVAL`.
12. `WITH RECURSIVE`, `LATERAL`, `FOR UPDATE`.
13. Dialect abstraction (placeholder style, identifier quoting) if non-PG targets matter.

**Explicitly out of scope** (worth writing into `docs/VISION.md`): DDL, transaction
control, `EXPLAIN`, `COPY`, permissions, and anything an ORM would do (mapping rows to
objects, identity maps, lazy loading).
