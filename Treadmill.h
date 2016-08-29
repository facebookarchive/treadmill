/*
 *  Copyright (c) 2014, Facebook, Inc.
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

#include "treadmill/Scheduler.h"
#include "treadmill/Worker.h"

// The path to the workload configuration file
DECLARE_string(config_file);

// The hostname of the server
DECLARE_string(hostname);

DECLARE_string(counter_name);

DECLARE_int32(counter_threshold);

// The number of connections each worker thread handles
DECLARE_int32(number_of_connections);

// The total number of workers
DECLARE_int32(number_of_workers);

// The number of keys in the workload
DECLARE_int64(number_of_keys);

// The port number to connect
DECLARE_int32(port);

// The request per second trying to send
DECLARE_int32(request_per_second);

// The total testing time in second
DECLARE_int32(runtime);

// The file to store the JSON output statistics
DECLARE_string(output_file);

// The max number of requests to have outstanding per worker
DECLARE_int32(max_outstanding_requests);

// Config filename to pass into the workload in JSON format
DECLARE_string(config_in_file);

// Config filename to export from the workload in JSON format
DECLARE_string(config_out_file);

// Comma-separated list of CPU IDs to pin the workers
DECLARE_string(cpu_affinity);

// Default number of calibration samples for continuous statistics
DECLARE_int32(default_calibration_samples);

// Default number of warm-up samples for continuous statistics
DECLARE_int32(default_warmup_samples);

// Number of calibration samples for latency statistics
DECLARE_int32(latency_calibration_samples);

// Number of warm-up samples for latency statistics
DECLARE_int32(latency_warmup_samples);

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

  StatisticsManager::printAll();
  if (FLAGS_output_file != "") {
    int fd = open(FLAGS_output_file.c_str(),
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0666);
    if (fd == -1) {
      LOG(FATAL) << "Open to write file failed: " << FLAGS_output_file;
    }
    std::string json = StatisticsManager::toJson();
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

void init(int argc, char* argv[]);

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
