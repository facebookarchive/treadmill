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

#include "treadmill/services/sleep/gen-cpp2/Sleep.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {
namespace services {
namespace sleep {

/**
 * A simple thrift server which has only one method that tells the server to
 * sleep for the amount of time embedded in the received request
 */
class SleepHandler : public SleepSvIf {
 public:
  SleepHandler() { }
  /**
   * Asynchronous function to handle goSleep request
   *
   * @param time The time to sleep in microseconds
   * @return Future to the actual time spent in sleep in microseconds
   */
  folly::Future<int64_t> future_goSleep(int64_t time);
};

} // namespace sleep
} // namespace services
} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
