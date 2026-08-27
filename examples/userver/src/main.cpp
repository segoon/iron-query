// A minimal userver HTTP service reading key/value pairs from PostgreSQL.
// The query is built with iron-query instead of a hand-written SQL string or
// a static `.sql` file, and handed to the driver via
// iron_query::userver::ToQuery. The WHERE and LIMIT clauses are added
// conditionally, demonstrating how to assemble a dynamic query with
// iron-query's move-only builder API. See ../static_config.yaml for the
// listening port/dbconnection and ../schemas/postgresql/schema.sql for the
// table.

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

/// @brief A key-value HTTP handler whose PostgreSQL statement is built with
/// iron-query, so a typo in a column name or a missing WHERE clause fails to
/// compile instead of failing on the server. The WHERE and LIMIT clauses are
/// added only when the matching query argument is present, showing how to
/// build up a query dynamically with iron-query's move-only builder.
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
    request.GetHttpResponse().SetContentType(http::content_type::kTextPlain);
    switch (request.GetMethod()) {
    case server::http::HttpMethod::kGet:
      return GetValues(request);
    default:
      throw server::handlers::ClientError(server::handlers::ExternalBody{
          fmt::format("Unsupported method {}", request.GetMethod())});
    }
  }

private:
  std::string GetValues(const server::http::HttpRequest &request) const {
    const auto &key = request.GetArg("key");
    const auto &limit_arg = request.GetArg("limit");

    auto select = iq::From(kKeyValueTable).Select({kKeyColumn, kValueColumn});
    if (!key.empty()) {
      select = std::move(select).Where(kKeyColumn == iq::_1);
    }
    if (!limit_arg.empty()) {
      select = std::move(select).Limit(std::stoi(limit_arg));
    }

    const auto query = iq::userver::ToQuery(
        select, storages::Query::Name{"iron_query_userver_select_values"});

    const auto result =
        key.empty()
            ? pg_cluster_->Execute(storages::postgres::ClusterHostType::kSlave,
                                   query)
            : pg_cluster_->Execute(storages::postgres::ClusterHostType::kSlave,
                                   query, key);

    std::string response;
    for (const auto &row : result) {
      const auto [row_key, row_value] = row.As<std::string, std::string>();
      response += fmt::format("{}={}\n", row_key, row_value);
    }
    return response;
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
