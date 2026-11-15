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

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "yb/util/metrics_writer.h"
#include "yb/util/test_macros.h"
#include "yb/util/test_util.h"

#include "yb/yql/pggate/webserver/ysql_conn_mgr_prometheus.h"

namespace yb::pggate {

namespace {

ConnectionStats MakePoolStats(const std::string& database, const std::string& user) {
  ConnectionStats stats = {};
  stats.active_clients = 1;
  strncpy(stats.database_name, database.c_str(), sizeof(stats.database_name) - 1);
  strncpy(stats.user_name, user.c_str(), sizeof(stats.user_name) - 1);
  return stats;
}

std::string PublishPools(const std::vector<ConnectionStats>& pools) {
  std::stringstream output;
  MetricPrometheusOptions opts;
  PrometheusWriter writer(&output, opts);
  MetricEntity::AttributeMap attr;
  PublishYsqlConnMgrPoolMetrics(&writer, &attr, pools);
  return output.str();
}

}  // namespace

class YsqlConnMgrPrometheusTest : public YBTest {};

// Regression test for #37028: unescaped quotes, backslashes and newlines in pool labels make the
// whole scrape unparseable.
TEST_F(YsqlConnMgrPrometheusTest, EscapesPoolLabels) {
  auto output = PublishPools({MakePoolStats("db\"q", "a\"b")});
  ASSERT_STR_CONTAINS(output, "user=\"a\\\"b\"");
  ASSERT_STR_CONTAINS(output, "database=\"db\\\"q\"");
  ASSERT_STR_NOT_CONTAINS(output, "user=\"a\"b\"");

  output = PublishPools({MakePoolStats("path\\to", "line1\nline2")});
  ASSERT_STR_CONTAINS(output, "database=\"path\\\\to\"");
  ASSERT_STR_CONTAINS(output, "user=\"line1\\nline2\"");
}

TEST_F(YsqlConnMgrPrometheusTest, PlainNamesUnchanged) {
  auto output = PublishPools({MakePoolStats("yugabyte", "yugabyte")});
  ASSERT_STR_CONTAINS(output, "ysql_conn_mgr_active_clients{");
  ASSERT_STR_CONTAINS(output, "user=\"yugabyte\"");
  ASSERT_STR_CONTAINS(output, "database=\"yugabyte\"");
}

}  // namespace yb::pggate
