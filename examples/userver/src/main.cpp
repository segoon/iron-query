// A minimal userver HTTP service storing key/value pairs in PostgreSQL.
// Every statement is built with iron-query instead of a hand-written SQL
// string or a static `.sql` file, and handed to the driver via
// iron_query::userver::ToQuery. See ../static_config.yaml for the listening
// port/dbconnection and ../schemas/postgresql/schema.sql for the table.

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utest/using_namespace_userver.hpp>
#include <userver/utils/daemon_run.hpp>

#include <iron_query/iron_query.hpp>
#include <iron_query/userver.hpp>

namespace samples::iron_query_userver {

namespace iq = iron_query;

namespace {

const iron_query::Table kKeyValueTable =
    iron_query::Table::FromRaw("key_value_table");
const iron_query::Column kKeyColumn{"key", "VARCHAR"};
const iron_query::Column kValueColumn{"value", "VARCHAR",
                                      /*is_nullable=*/true};

} // namespace

/// @brief A key-value HTTP handler whose PostgreSQL statements are all built
/// with iron-query, so a typo in a column name or a missing WHERE clause
/// fails to compile instead of failing on the server.
class KeyValue final : public server::handlers::HttpHandlerBase {
public:
  static constexpr std::string_view kName = "handler-key-value";

  KeyValue(const components::ComponentConfig &config,
           const components::ComponentContext &context)
      : HttpHandlerBase(config, context),
        pg_cluster_(
            context.FindComponent<components::Postgres>("key-value-database")
                .GetCluster()) {}

  std::string HandleRequest(server::http::HttpRequest &request,
                            server::request::RequestContext &) const override {
    const auto &key = request.GetArg("key");
    if (key.empty()) {
      throw server::handlers::ClientError(
          server::handlers::ExternalBody{"No 'key' query argument"});
    }

    request.GetHttpResponse().SetContentType(http::content_type::kTextPlain);
    switch (request.GetMethod()) {
    case server::http::HttpMethod::kGet:
      return GetValue(key, request);
    case server::http::HttpMethod::kPost:
      return PostValue(key, request.GetArg("value"), request);
    case server::http::HttpMethod::kDelete:
      return DeleteValue(key);
    default:
      throw server::handlers::ClientError(server::handlers::ExternalBody{
          fmt::format("Unsupported method {}", request.GetMethod())});
    }
  }

private:
  std::string GetValue(std::string_view key,
                       const server::http::HttpRequest &request) const {
    using namespace iron_query;

    const auto query = iq::userver::ToQuery(
        From(kKeyValueTable).Select(kValueColumn).Where(kKeyColumn == _1),
        storages::Query::Name{"iron_query_userver_select_value"});

    const auto result = pg_cluster_->Execute(
        storages::postgres::ClusterHostType::kSlave, query, key);
    if (result.IsEmpty()) {
      request.SetResponseStatus(server::http::HttpStatus::kNotFound);
      return {};
    }
    return result.AsSingleRow<std::string>();
  }

  std::string PostValue(std::string_view key, std::string_view value,
                        const server::http::HttpRequest &request) const {
    using namespace iron_query;

    const auto query = iq::userver::ToQuery(
        InsertInto(kKeyValueTable)
            .Columns({kKeyColumn, kValueColumn})
            .Values({_1, _2}),
        storages::Query::Name{"iron_query_userver_insert_value"});

    pg_cluster_->Execute(storages::postgres::ClusterHostType::kMaster, query,
                         key, value);
    request.SetResponseStatus(server::http::HttpStatus::kCreated);
    return std::string{value};
  }

  std::string DeleteValue(std::string_view key) const {
    using namespace iron_query;

    const auto query = iq::userver::ToQuery(
        DeleteFrom(kKeyValueTable).Where(kKeyColumn == _1),
        storages::Query::Name{"iron_query_userver_delete_value"});

    const auto result = pg_cluster_->Execute(
        storages::postgres::ClusterHostType::kMaster, query, key);
    return std::to_string(result.RowsAffected());
  }

  storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace samples::iron_query_userver

int main(int argc, char *argv[]) {
  const auto component_list =
      components::MinimalServerComponentList()
          .Append<samples::iron_query_userver::KeyValue>()
          .Append<components::Postgres>("key-value-database")
          .AppendComponentList(clients::http::ComponentList())
          .Append<components::TestsuiteSupport>()
          .Append<server::handlers::TestsControl>()
          .Append<clients::dns::Component>();
  return utils::DaemonMain(argc, argv, component_list);
}
