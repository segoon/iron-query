// A minimal example wiring iron-query directly to raw libpq, with no
// framework in between: the SQL text comes from `SelectExpr`/`InsertInto`'s
// `.ToString()`, and bind values are handed to `PQexecParams` positionally,
// matching iron-query's `$1`/`$2`... (`iq::_1`/`iq::_2`) placeholder
// convention. See ../schema.sql for the table and ../../userver/src/main.cpp
// for the same query built for userver's Postgres driver instead.
//
// Usage: iron-query-libpq-example [conninfo]
// With no argument, libpq falls back to the standard PG* environment
// variables (PGHOST, PGUSER, PGDATABASE, ...).

#include <iron_query/iron_query.hpp>

#include <libpq-fe.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace iq = iron_query;

namespace {

const iq::Table kKeyValueTable = iq::Table::FromRaw("key_value_table");
const iq::Column kKeyColumn{"key", "VARCHAR"};
const iq::Column kValueColumn{"value", "VARCHAR", /*is_nullable=*/true};

// Runs `sql` with `params` bound positionally to $1, $2, ..., aborting the
// program with libpq's error message on failure.
PGresult *ExecParams(PGconn *conn, const std::string &sql,
                     const std::vector<const char *> &params) {
  PGresult *result =
      PQexecParams(conn, sql.c_str(), static_cast<int>(params.size()),
                   /*paramTypes=*/nullptr, params.data(),
                   /*paramLengths=*/nullptr, /*paramFormats=*/nullptr,
                   /*resultFormat=*/0);
  const auto status = PQresultStatus(result);
  if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
    std::cerr << "query failed: " << PQresultErrorMessage(result) << '\n';
    PQclear(result);
    PQfinish(conn);
    std::exit(EXIT_FAILURE);
  }
  return result;
}

} // namespace

int main(int argc, char *argv[]) {
  const char *conninfo = argc > 1 ? argv[1] : "";
  PGconn *conn = PQconnectdb(conninfo);
  if (PQstatus(conn) != CONNECTION_OK) {
    std::cerr << "connection failed: " << PQerrorMessage(conn) << '\n';
    PQfinish(conn);
    return EXIT_FAILURE;
  }

  const auto insert = iq::InsertInto(kKeyValueTable)
                          .Columns({kKeyColumn, kValueColumn})
                          .Values({iq::_1, iq::_2})
                          .ToString();
  PGresult *insert_result =
      ExecParams(conn, insert, {"greeting", "hello, libpq"});
  PQclear(insert_result);

  const auto select = iq::From(kKeyValueTable)
                          .Select({kKeyColumn, kValueColumn})
                          .Where(kKeyColumn == iq::_1)
                          .ToString();
  PGresult *select_result = ExecParams(conn, select, {"greeting"});

  for (int row = 0; row < PQntuples(select_result); ++row) {
    std::cout << PQgetvalue(select_result, row, 0) << '='
              << PQgetvalue(select_result, row, 1) << '\n';
  }

  PQclear(select_result);
  PQfinish(conn);
  return EXIT_SUCCESS;
}
