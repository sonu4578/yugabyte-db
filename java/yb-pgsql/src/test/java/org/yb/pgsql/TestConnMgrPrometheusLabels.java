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
//
package org.yb.pgsql;

import static org.yb.AssertionWrappers.assertFalse;
import static org.yb.AssertionWrappers.assertTrue;

import java.net.URL;
import java.sql.Connection;
import java.sql.Statement;
import java.util.Scanner;
import org.junit.Assume;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.yb.YBTestRunner;
import org.yb.minicluster.MiniYBDaemon;

/**
 * Verifies that Connection Manager pool metrics on the YSQL webserver's /prometheus-metrics
 * endpoint escape the database and user labels (GitHub issue #37028).
 */
@RunWith(value = YBTestRunner.class)
public class TestConnMgrPrometheusLabels extends BasePgSQLTest {
  private static final String ROLE_WITH_QUOTE = "a\"b";

  @Override
  protected java.util.Map<String, String> getTServerFlags() {
    java.util.Map<String, String> flags = super.getTServerFlags();
    if (isTestRunningWithConnectionManager()) {
      flags.put("enable_ysql_conn_mgr", "true");
    }
    return flags;
  }

  @Test
  public void testPoolMetricLabelsAreEscaped() throws Exception {
    Assume.assumeTrue(isTestRunningWithConnectionManager());

    try (Statement stmt = connection.createStatement()) {
      stmt.execute("CREATE ROLE \"a\"\"b\" LOGIN");
    }
    // Create the pool for the role by connecting through Connection Manager.
    try (Connection conn = getConnectionBuilder().withUser(ROLE_WITH_QUOTE).connect();
         Statement stmt = conn.createStatement()) {
      stmt.execute("SELECT 1");

      boolean found = false;
      for (MiniYBDaemon ts : miniCluster.getTabletServers().values()) {
        URL url = new URL(String.format("http://%s:%d/prometheus-metrics",
                                        ts.getLocalhostIP(), ts.getPgsqlWebPort()));
        try (Scanner scanner = new Scanner(url.openConnection().getInputStream())) {
          while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (!line.startsWith("ysql_conn_mgr_")) {
              continue;
            }
            // The unescaped form would break the exposition format.
            assertFalse("Unescaped label in: " + line, line.contains("user=\"a\"b\""));
            if (line.contains("user=\"a\\\"b\"")) {
              found = true;
            }
          }
        }
      }
      assertTrue("No ysql_conn_mgr_* metric with escaped user label found", found);
    }
  }
}
