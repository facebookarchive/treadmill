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

// The request per second trying to send
DEFINE_int32(request_per_second,
             1024,
             "The request per second to send.");

// The total testing time in second
DEFINE_int32(runtime,
             120,
             "The total runtime in seconds.");

// The file to store the JSON output statistics
DEFINE_string(output_file,
              "",
              "Where to print the json output of statistics.");

// The max number of requests to have outstanding per worker
DEFINE_int32(max_outstanding_requests,
             1000,
             "The max number of requests to have outstanding per worker.");

// Config filename to pass into the workload in JSON format
DEFINE_string(config_in_file,
              "",
              "Config filename to pass into the workload in JSON format.");

// Config filename to export from the workload in JSON format
DEFINE_string(config_out_file,
              "",
              "Config filename to export from the workload in JSON format.");

// Comma-separated list of CPU IDs to pin the workers
DEFINE_string(cpu_affinity,
              "",
              "Comma-separated list of CPU IDs to pin the workers.");

// Default number of calibration samples for continuous statistics
DEFINE_int32(default_calibration_samples,
             10,
             "Default number of calibration samples for continuous statistics.");

// Default number of warm-up samples for continuous statistics
DEFINE_int32(default_warmup_samples,
             10,
             "Default number of warm-up samples for continuous statistics.");

// Number of calibration samples for latency statistics
DEFINE_int32(latency_calibration_samples,
             1000,
             "Number of calibration samples for latency statistics.");

// Number of warm-up samples for latency statistics
DEFINE_int32(latency_warmup_samples,
             1000,
             "Number of warm-up samples for latency statistics.");

namespace facebook {
namespace windtunnel {
namespace treadmill {

void init(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  // Set the usage information
  std::string usage("Treadmill loadtester");
  google::SetUsageMessage(usage);
  // Parse all the command line flags
  google::ParseCommandLineFlags(&argc, &argv, true);
}

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
