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

#include <memory>
#include <thread>

#include <folly/io/async/NotificationQueue.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

class Scheduler {
 public:
  Scheduler(uint32_t rps, uint32_t number_of_workers,
            uint32_t logging_threshold);
  ~Scheduler();

  void run();
  void stopAndJoin();

  folly::NotificationQueue<int>& getWorkerQueue(uint32_t id);
 private:
  /**
   * Draws from an exponential distribution with the given mean.
   */
  static double randomExponentialInterval(double mean);

  /**
   * Waits until given amount of nanosecond pases, for precise timing it uses
   * spin-loop.
   */
  static void waitNs(int64_t ns);

  void loop();

  uint32_t logging_threshold_;
  uint32_t next_{0};
  uint32_t rps_;

  std::vector<uint64_t> logged_;
  std::vector<folly::NotificationQueue<int>> queues_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> thread_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
