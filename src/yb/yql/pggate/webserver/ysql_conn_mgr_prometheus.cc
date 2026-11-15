// Copyright (c) YugabyteDB, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied.  See the License for the specific language governing permissions and limitations
// under the License.

#include "yb/yql/pggate/webserver/ysql_conn_mgr_prometheus.h"

#include "yb/util/status_log.h"

namespace yb::pggate {

namespace {

const char* const kDatabaseLabel = "database";
const char* const kUserLabel = "user";
const char* const kMetricTypeServer = "server";

struct YsqlConnMgrMetric {
  const char* name;
  uint64_t value;
  const char* type;
  const char* help;
};

}  // namespace

void PublishYsqlConnMgrPoolMetrics(
    PrometheusWriter* pwriter, MetricEntity::AttributeMap* attr,
    const std::vector<ConnectionStats>& stats_list) {
  std::vector<YsqlConnMgrMetric> ysql_conn_mgr_metrics;
  for (const auto& stats : stats_list) {
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_active_clients", stats.active_clients, "gauge",
         "Number of active logical connections"});
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_queued_clients", stats.queued_clients, "gauge",
         "Number of logical connections waiting in the queue for a physical connection"});
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_waiting_clients", stats.waiting_clients, "gauge",
         "Number of logical connections that are either idle (i.e. no ongoing transaction) or "
         "waiting for the worker thread to be processed"});
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_active_servers", stats.active_servers, "gauge",
         "Number of physical connections that are currently attached to a logical connection."});
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_idle_servers", stats.idle_servers, "gauge",
         "Number of physical connections that are not attached to any logical connection"});
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_query_rate", stats.query_rate, "gauge",
         "Query rate for last <stats_interval> (set in odyssey config) period of time."});
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_transaction_rate", stats.transaction_rate, "gauge",
         "Transaction rate for last <stats_interval> (set in odyssey config) period of time"});
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_avg_wait_time_ns", stats.avg_wait_time_ns, "gauge",
         "Avg wait time (in nanoseconds) for a logical connection to be attached to a physical "
         "connection"});
    ysql_conn_mgr_metrics.push_back(
        {"ysql_conn_mgr_sticky_connections", stats.sticky_connections, "gauge",
         "Number of logical connections attached to a physical connection for the lifetime of the "
         "logical connection"});
    (*attr)[kDatabaseLabel] = EscapePrometheusLabelValue(stats.database_name);
    (*attr)[kUserLabel] = EscapePrometheusLabelValue(stats.user_name);

    // Publish collected metrics for the current pool.
    for (auto entry : ysql_conn_mgr_metrics) {
      WARN_NOT_OK(
          pwriter->WriteSingleEntry(
              *attr, entry.name, entry.value, AggregationFunction::kSum,
              kServerLevel, kMetricTypeServer, entry.type, entry.help),
          "Cannot publish Ysql Connection Manager metric to Prometheus-metrics endpoint");
    }
    // Clear the collected metrics for the metrics collected for the next pool.
    ysql_conn_mgr_metrics.clear();
  }
}

}  // namespace yb::pggate
