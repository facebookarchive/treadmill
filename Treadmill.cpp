/*
* Copyright (c) 2013, Facebook, Inc.
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*   * Neither the name Facebook nor the names of its contributors may be used to
*     endorse or promote products derived from this software without specific
*     prior written permission.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "Distribution.h"
#include "Request.h"
#include "Worker.h"
#include "Workload.h"

using std::shared_ptr;
using std::string;
using std::vector;

// The path to the workload configuration file
DEFINE_string(config_file,
              "./examples/flat.json",
              "The path to the workload configuration file");
// The hostname of the server
DEFINE_string(hostname,
              "localhost",
              "The host to load test.");
// The number of connections each worker thread handles
DEFINE_int32(number_of_connections,
             4,
             "The number of connections for each thread worker");
// The total number of workers
DEFINE_int32(number_of_workers,
             1,
             "The number of workers");
// The number of keys in the workload
DEFINE_int64(number_of_keys,
             1024,
             "The number of keys in the workload");
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
// The request type used for warm-up
DEFINE_string(warmup_request_type,
              "MemcachedSetRequest",
              "The request type used for warm-up.");
// The workload to test
DEFINE_string(workload_type,
              "Memcached",
              "The workload type to test.");

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Press start to rock
 *
 * @param argc Argument count
 * @param argv Argument vector
 */
int run(int argc, char* argv[]) {
  shared_ptr<Workload> workload =
    Workload::generateWorkloadByConfigFile(FLAGS_number_of_keys,
                                           FLAGS_config_file);

  // Initialize the map of request types in the workload
  RequestTypeFactory::initializeRequestTypesByWorkload(FLAGS_workload_type);
  vector<string> request_types_in_workload =
    RequestTypeFactory::request_types_in_workload();

  // Initialize statistic
  shared_ptr<Statistic> statistic = shared_ptr<Statistic>(new Statistic());
  statistic->addStatistic(kAllTypesOfRequest);
  for (int i = 0; i < request_types_in_workload.size(); i++) {
    statistic->addStatistic(request_types_in_workload[i]);
  }

  // Initialize the interarrival rate distribution
  double interarrival_rate =
    (1e6 * (double)FLAGS_number_of_workers *
     (double)FLAGS_number_of_connections) / (double) FLAGS_request_per_second;
  shared_ptr<ExponentialDistribution> interarrival_distribution =
    shared_ptr<ExponentialDistribution>(
        new ExponentialDistribution(interarrival_rate));

  vector<Worker> workers;
  // Initialize
  for (int i = 0; i < FLAGS_number_of_workers; i++) {
    workers.push_back(Worker(workload, statistic, interarrival_distribution,
                             i, FLAGS_number_of_workers));
  }

  // Warm-Up
  for (int i = 0; i < FLAGS_number_of_workers; i++) {
    workers[i].warmUp();
  }
  LOG(INFO) << "Warm-Up";

  // Start testing
  for (int i = 0; i < FLAGS_number_of_workers; i++) {
    workers[i].start();
  }

  // Wait the test to finish
  sleep(FLAGS_runtime);

  // Stop testing
  for (int i = 0; i < FLAGS_number_of_workers; i++) {
    workers[i].stop();
  }
  LOG(INFO) << "Complete";

  statistic->printStatistic();

  return 0;
}

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook

/**
 * Entry function for treadmill
 *
 * @param argc Argument count
 * @param argv Argument vector
 */
int main(int argc, char* argv[]) {
  // Set the usage information
  std::string usage("This program does nothing. Sample usage:\n ./treadmill");
  google::SetUsageMessage(usage);
  // Set the version information
  google::SetVersionString("0.1");
  // Parse all the command line flags
  google::ParseCommandLineFlags(&argc, &argv, true);
  // Start treadmill
  return facebook::windtunnel::treadmill::run(argc, argv);
}
