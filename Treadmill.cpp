/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <string>

#include <folly/init/Init.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

// The path to the workload configuration file
DEFINE_string(config_file,
              "./examples/flat.json",
              "The path to the workload configuration file.");

// The hostname of the server
DEFINE_string(hostname,
              "localhost",
              "The host to load test.");

DEFINE_bool(wait_for_target_ready,
            false,
            "If true, wait until the target is ready.");

DEFINE_string(counter_name,
              "",
              "Counter to compare against threshold (e.g, foo.bar.count).");

DEFINE_int32(counter_threshold,
             -1,
             "counter_name value > counter_threshold before sending requests.");

// The number of connections each worker thread handles
DEFINE_int32(number_of_connections,
             4,
             "The number of connections for each thread worker.");

// The total number of workers
DEFINE_int32(number_of_workers,
             1,
             "The number of workers.");

// The number of keys in the workload
DEFINE_int64(number_of_keys,
             1024,
             "The number of keys in the workload.");

// The port number to connect
DEFINE_int32(port,
             11211,
             "The port on the host to connect to.");

DEFINE_int32(control_port,
             23456,
             "Port for TreadmillService remote control.");

// The request per second trying to send
DEFINE_int32(request_per_second,
             1024,
             "The request per second to send.");

// The total testing time in second
DEFINE_int32(runtime,
             120,
             "The total runtime in seconds.");

// The max number of requests to have outstanding per worker
DEFINE_int32(max_outstanding_requests,
             1000,
             "The max number of requests to have outstanding per worker.");

// Config filename to pass into the workload in JSON format
DEFINE_string(config_in_file,
              "",
              "Config filename to pass into the workload in JSON format.");

DEFINE_string(config_in_json,
              "",
              "Configuration string to be parsed as JSON for the workload. "
              "If --config_in_file is also specified, "
              "the configs are merged.");

// Config filename to export from the workload in JSON format
DEFINE_string(config_out_file,
              "",
              "Config filename to export from the workload in JSON format.");

// Comma-separated list of CPU IDs to pin the workers
DEFINE_string(cpu_affinity,
              "",
              "Comma-separated list of CPU IDs to pin the workers.");

DEFINE_int32(server_port, -1, "Port for fb303 server");

DEFINE_int32(worker_shutdown_delay,
             1,
             "Seconds to allow for workers to gracefully shutdown");

namespace facebook {
namespace windtunnel {
namespace treadmill {

void init(int argc, char* argv[]) {
  // Set the usage information
  std::string usage("Treadmill loadtester");
  gflags::SetUsageMessage(usage);

  folly::init(&argc, &argv);
}

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
