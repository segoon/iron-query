# Schema generator: discovery and proposal

Closes `docs/SQL.md` §5 P2 item 7 ("schema→`TableWithColumns` codegen") and the
`docs/TODO.md` items "generate table vars from actual DB" and "userver sql2dto".

## Prior art surveyed

### userver's `scripts/sqldto`

The most directly relevant prior art: same reference dialect (PostgreSQL), same
underlying problem (schema safety for query building). Source:
`userver/scripts/sqldto`.

Two-stage pipeline:

1. **Dumper** (dev-time, needs a live Postgres) — replays the project's real
   migration `.sql` files against a throwaway Postgres, then introspects
   `pg_catalog` (`pg_attribute`, `pg_class`, `pg_namespace`, plus `pg_type`/
   `pg_enum` for enums/composites) for column names, types, and nullability
   (`attnotnull`). Writes a **committed JSON dump** (`schema.dto.json`) tagged
   with a sha256 hash of the migration + query source text.
2. **Generator** (build-time, offline) — reads only the committed dump and
   Jinja templates, never touches a DB. If migrations changed since the dump
   was taken, the hash mismatches and the build fails with the exact refresh
   command to run. Normal builds/CI stay DB-free.

Query result/param types are derived by actually `PREPARE`/`EXECUTE`-ing each
query inside a rolled-back transaction (probing nullability by retrying with
`NULL` args) — avoids writing a SQL type-inference engine, but is far more
scope than table-schema generation alone: it also generates per-query result
structs, enum/composite types, and a full typed client + gmock mock.

### The rest of `docs/ALTERNATIVES.md`

Three camps, by where the schema's source of truth lives:

| Camp | Members | Mechanism |
|---|---|---|
| Live-DB introspection | userver, jOOQ (default) | Catalog/`information_schema` query against a real or ephemeral DB |
| Static DDL-text parsing, no DB | sqlpp11/23 (`ddl2cpp`), sqlc (DDL mode) | Own grammar (sqlpp) or embedded real parser, `libpg_query` (sqlc) |
| Code-first (opposite direction) | ent, sqlite_orm, ODB, sqlgen | Schema is hand-written in the host language (structs/reflection); DB DDL is *generated from it*, not read from it |

jOOQ straddles the first two: live JDBC introspection by default, with a
DDL-replay-into-H2 fallback for when a persistent DB isn't wanted —
conceptually identical to userver's throwaway-Postgres dumper.

The code-first camp (ent, sqlite_orm, ODB, sqlgen) doesn't close IQ's actual
gap ("lost field after migration") because the struct/model is still the
hand-maintained source of truth; only DB-first tools (userver, jOOQ, sqlpp11/23,
sqlc) actually derive code from the real schema.

## Proposal for IQ

Mirror userver's **dump/generate split**, scoped down to just
`TableWithColumns` generation, as a small standalone Python tool:

1. **`iq-schema-dump`** (dev-time, needs Postgres) — given an ordered
   `migrations/*.sql` directory, applies them to a real/ephemeral Postgres,
   introspects `pg_catalog` for table/column/type/nullability, writes a
   committed `schema.iq.json` gated by a sha256 hash of the migration sources.
2. **`iq-schema-generate`** (build-time, offline) — reads only
   `schema.iq.json`, emits `include/<ns>/tables.hpp` with one
   `inline const TableWithColumns k<Table>` per table, using the existing
   `Column{name, type, is_nullable}` API — literally what a developer writes
   by hand today, just generated. A CMake helper
   (`iq_add_table_schema(<target> MIGRATIONS_DIR ... DUMP_DIR ...)`) checks
   the hash and fails with the refresh command if stale, so normal
   builds/CI stay DB-free and the dump is committed like a lockfile.

**Why live introspection over static DDL parsing**: Postgres is already IQ's
one true reference dialect (`docs/SQL.md`), so reimplementing/tracking PG's
DDL grammar (domains, extensions, generated columns, etc.) buys nothing —
`pg_catalog` gives ground truth for free, and the hash-gated dump gets
sqlc-style "CI needs no DB" ergonomics without a parser.

**Scope**: intentionally much narrower than userver's tool. No query-result-
struct generation, no client/mock generation, no enum/composite structs
(those belong to separate P3/type-system TODO items) — just closes the
"no schema→`TableWithColumns` generation" gap.
