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

#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/io/async/NotificationQueue.h>

DECLARE_bool(wait_for_runner_ready);

namespace facebook {
namespace windtunnel {
namespace treadmill {

class Scheduler {
 public:
  Scheduler(uint32_t rps, uint32_t number_of_workers,
            uint32_t logging_threshold);
  ~Scheduler();

  folly::Future<folly::Unit> run();

  // Transition from running to paused (no-op if not running).
  void pause();

  // Transition from paused to running (no-op if not paused).
  void resume();

  // It is safe to call stop() multiple times.
  void stop();

  // The scheduler _must_ be stopped first.
  void join();

  folly::NotificationQueue<int>& getWorkerQueue(uint32_t id);
 private:
  enum RunState { RUNNING, PAUSED, STOPPING };

  /**
   * Draws from an exponential distribution with the given mean.
   */
  static double randomExponentialInterval(double mean);

  /**
   * Waits until given amount of nanosecond pases, for precise timing it uses
   * spin-loop.
   */
  static void waitNs(int64_t ns);

  /**
   * Puts given message on each worker's queue.
   */
  void messageAllWorkers(int message);

  void loop();

  uint32_t logging_threshold_;
  uint32_t next_{0};
  uint32_t rps_;

  std::vector<uint64_t> logged_;
  std::vector<folly::NotificationQueue<int>> queues_;
  std::atomic<RunState> state_;
  std::unique_ptr<std::thread> thread_;
  folly::Promise<folly::Unit> promise_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
