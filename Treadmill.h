/*
 *  Copyright (c) 2016, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once

#include <string>
#include <thread>
#include <vector>

#include <folly/String.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "Scheduler.h"
#include "Worker.h"

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
              "Counter to compare against threshold (e.g., foo.bar.count).");

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

DEFINE_string(output_file,
              "",
              "Where to print the json output of statistics.");

DEFINE_int32(max_outstanding_requests,
             1000,
             "The max number of requests to have outstanding per worker.");

DEFINE_string(config_in_file,
              "",
              "Config filename to pass into the workload in JSON format.");

DEFINE_string(config_out_file,
              "",
              "Config filename to export from the workload in JSON format.");

DEFINE_string(cpu_affinity,
              "",
              "Comma-separated list of CPU IDs to pin the workers.");

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Press start to rock
 *
 * @param argc Argument count
 * @param argv Argument vector
 */
template <typename Service>
int run(int argc, char* argv[]) {
  StatisticsManager& statistics_manager = StatisticsManager::get();
  std::vector<std::unique_ptr<Worker<Service>>> workers;
  double rps = FLAGS_request_per_second / (double) FLAGS_number_of_workers;
  int max_outstanding_requests_per_worker =
    FLAGS_max_outstanding_requests / FLAGS_number_of_workers;
  LOG(INFO) << "Desired rps per worker: " << rps;
  LOG(INFO) << "Max outstanding requests per worker: "
            << max_outstanding_requests_per_worker;
  LOG(INFO) << "N Workers: " << FLAGS_number_of_workers;
  LOG(INFO) << "N Connections: " << FLAGS_number_of_connections;

  folly::dynamic config = folly::dynamic::object;
  if (FLAGS_config_in_file != "") {
    config = readDynamicFromFile(FLAGS_config_in_file);
  }
  int cpu_affinity_list[FLAGS_number_of_workers];
  std::fill_n(cpu_affinity_list, FLAGS_number_of_workers, -1);

  if (FLAGS_cpu_affinity != "") {
    int total_number_of_cores = std::thread::hardware_concurrency();
    std::vector<folly::StringPiece> affinity_string_list;
    folly::split(",", FLAGS_cpu_affinity, affinity_string_list);
    if (affinity_string_list.size() != FLAGS_number_of_workers) {
      LOG(FATAL) << "Length of the CPU affinity list ("
                 << affinity_string_list.size()
                 << ") does not match the number of workers ("
                 << FLAGS_number_of_workers << ")";
    } else {
      for (int i = 0; i < FLAGS_number_of_workers; i++) {
        int cpu_affinity = folly::to<int>(affinity_string_list[i]);
        if (cpu_affinity >= 0 && cpu_affinity < total_number_of_cores) {
          cpu_affinity_list[i] = cpu_affinity;
        } else {
          LOG(FATAL) << "Core " << cpu_affinity << " does not exist";
        }
      }
    }
  }

  Scheduler scheduler(FLAGS_request_per_second,
                      FLAGS_number_of_workers,
                      max_outstanding_requests_per_worker);
  for (int i = 0; i < FLAGS_number_of_workers; i++) {
    workers.push_back(folly::make_unique<Worker<Service>>(
                        scheduler.getWorkerQueue(i),
                        FLAGS_number_of_workers,
                        FLAGS_number_of_connections,
                        max_outstanding_requests_per_worker,
                        config,
                        cpu_affinity_list[i]
                      )
    );
  }

  // Start testing
  for (int i = 0; i < FLAGS_number_of_workers; i++) {
    workers[i]->run();
  }

  scheduler.run();

  // Wait the test to finish
  sleep(FLAGS_runtime);

  statistics_manager.printAll();
  if (FLAGS_output_file != "") {
    int fd = open(FLAGS_output_file.c_str(),
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0666);
    if (fd == -1) {
      LOG(FATAL) << "Open to write file failed: " << FLAGS_output_file;
    }
    std::string json = statistics_manager.toJson();
    writeBlock(fd, json.c_str(), json.size());
    close(fd);
  }

  LOG(INFO) << "Stopping and joining scheduler thread";

  scheduler.stopAndJoin();

  LOG(INFO) << "Stopping workers";

  // We already stored stats, so just drop all remaining scheduled request.
  for (int i = 0; i < FLAGS_number_of_workers; i++) {
    workers[i]->stop();
  }

  LOG(INFO) << "Joining worker threads";

  // Join worker threads
  for (int i = 0; i < FLAGS_number_of_workers; i++) {
    workers[i]->join();
  }
  if (FLAGS_config_out_file != "") {
    LOG(INFO) << "Saving config";
    std::vector<Worker<Service>*> workerRefs;
    for (auto& worker: workers) {
      workerRefs.push_back(worker.get());
    }
    auto config_output = workers[0]->makeConfigOutputs(workerRefs);
    writeDynamicToFile(FLAGS_config_out_file, config_output);
  }

  LOG(INFO) << "Complete";

  return 0;
}

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
