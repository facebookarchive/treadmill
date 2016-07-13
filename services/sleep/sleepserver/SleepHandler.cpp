/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "treadmill/services/sleep/sleepserver/SleepHandler.h"

#include <unistd.h>

#include <folly/MoveWrapper.h>
#include <folly/futures/Future.h>

using namespace folly;

namespace facebook {
namespace windtunnel {
namespace treadmill {
namespace services {
namespace sleep {

/**
 * Asynchronous function to process goSleep request
 *
 * @param time The time to sleep in microseconds
 * @return Future to the actual time spent in sleep in microseconds
 */
Future<int64_t> SleepHandler::future_goSleep(int64_t time) {
  folly::MoveWrapper<Promise<int64_t>> promise;
  auto future = promise->getFuture();

  folly::RequestEventBase::get()->runInEventBaseThread(
      [promise, time]() mutable {
        struct timeval start_time, end_time, diff_time;;
        gettimeofday(&start_time, nullptr);
        usleep(time);
        gettimeofday(&end_time, nullptr);
        timersub(&end_time, &start_time, &diff_time);
        promise->setValue(diff_time.tv_sec * 1e6 + diff_time.tv_usec);
      });

  return future;
}

} // namespace sleep
} // namespace services
} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
