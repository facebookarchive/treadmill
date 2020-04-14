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

#include <vector>

#include "treadmill/services/sleep/SleepService.h"

#include "treadmill/Workload.h"

DECLARE_int64(sleep_time);

namespace facebook {
namespace windtunnel {
namespace treadmill {

template <>
class Workload<SleepService> : public WorkloadBase<Workload<SleepService>> {
 public:
  Workload<SleepService>(folly::dynamic /*config*/) {}

  void reset() {}

  std::tuple<
      std::unique_ptr<SleepService::Request>,
      folly::Promise<SleepService::Reply>,
      folly::Future<SleepService::Reply>>
  getNextRequest() {
    std::unique_ptr<SleepService::Request> request =
        std::make_unique<SleepRequest>(SleepRequest::SLEEP, FLAGS_sleep_time);
    folly::Promise<SleepService::Reply> p;
    auto f = p.getFuture();
    return std::make_tuple(std::move(request), std::move(p), std::move(f));
  }

  folly::dynamic makeConfigOutputs(
      std::vector<Workload<SleepService>*> /*workloads*/) {
    return folly::dynamic::object;
  }
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
