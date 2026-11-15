# Copyright (c) YugabyteDB, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
# in compliance with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under the License
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
# or implied.  See the License for the specific language governing permissions and limitations
# under the License.
#

set(YB_PCH_PATH ../)

# Kept separate from yb_pggate_webserver so it can be unit tested without postgres.
ADD_YB_LIBRARY(yb_pggate_conn_mgr_metrics
               SRCS ysql_conn_mgr_prometheus.cc
               DEPS yb_util)

set(PGGATE_WEBSERVER_SRCS
    ybc_pg_webserver_wrapper.cc
    )

set(PGGATE_WEBSERVER_LIBS
    server_process
    yb_common
    yb_pggate_conn_mgr_metrics
    yb_pggate_util
    yb_util
    )

ADD_YB_LIBRARY(yb_pggate_webserver
               SRCS ${PGGATE_WEBSERVER_SRCS}
               DEPS ${PGGATE_WEBSERVER_LIBS})

set(YB_TEST_LINK_LIBS yb_pggate_conn_mgr_metrics ${YB_MIN_TEST_LIBS})
ADD_YB_TEST(ysql_conn_mgr_prometheus-test)
