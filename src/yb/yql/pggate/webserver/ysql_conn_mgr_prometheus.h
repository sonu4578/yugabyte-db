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

#pragma once

#include <cstdint>
#include <vector>

#include "yb/util/metric_entity.h"
#include "yb/util/metrics_writer.h"

#include "yb/yql/ysql_conn_mgr_wrapper/ysql_conn_mgr_stats.h"

namespace yb::pggate {

// Publishes the per-pool ysql_conn_mgr_* metrics, labelled with the pool's database and user.
// `attr` supplies the common labels; its database and user entries are overwritten per pool.
void PublishYsqlConnMgrPoolMetrics(
    PrometheusWriter* pwriter, MetricEntity::AttributeMap* attr,
    const std::vector<ConnectionStats>& stats_list);

}  // namespace yb::pggate
