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

#include <folly/futures/Future.h>

#include <mcrouter/lib/cycles/Cycles.h>

#include "treadmill/StatisticsManager.h"
#include "treadmill/Workload.h"

DECLARE_int64(number_of_keys);
DECLARE_string(libmcrouter_keys_prefix);

namespace facebook {
namespace windtunnel {
namespace treadmill {

template <>
class Workload<LibmcrouterService> {
 public:
  enum class State {
    WARMUP,
    GETS
  };

  explicit Workload(folly::dynamic config)
    : state_(State::WARMUP),
      index_(0) {
    cycles_statistic_ = &StatisticsManager::get()
      .getContinuousStat("cpu_cycles");
    facebook::memcache::cycles::startExtracting(
        [this](facebook::memcache::cycles::CycleStats stats) {
          if (stats.numSamples > 0) {
            cycles_statistic_->addSample(stats.avg);
          }
        }
    );
  }

  std::tuple<std::unique_ptr<LibmcrouterService::Request>,
             folly::Promise<LibmcrouterService::Reply>,
             folly::Future<LibmcrouterService::Reply>>
  getNextRequest() {
    if (index_ == FLAGS_number_of_keys) {
      index_ = 0;
    }

    sprintf(buffer, "%s%d", FLAGS_libmcrouter_keys_prefix.c_str(), index_);
    std::string key = buffer;

    std::unique_ptr<LibmcrouterService::Request> request;
    if (state_ == State::WARMUP) {
      sprintf(buffer, "Value:%d.%s", index_,
              FLAGS_libmcrouter_keys_prefix.c_str());
      std::string value = buffer;
      request = folly::make_unique<LibmcrouterService::Request>(
        facebook::memcache::MemcacheClientString::UpdateRequests(
          {{std::move(key), std::move(value)}}));

      if (index_ == FLAGS_number_of_keys - 1) {
        LOG(INFO) << "WARMUP complete";
        state_ = State::GETS;
      }
    } else if (state_ == State::GETS) {
      request = folly::make_unique<LibmcrouterService::Request>(
          facebook::memcache::MemcacheClientString::GetRequests(
              {{std::move(key)}}));
    }
    folly::Promise<LibmcrouterService::Reply> p;
    auto f = p.getFuture();
    ++index_;
    return std::make_tuple(std::move(request), std::move(p), std::move(f));
  }

  folly::dynamic makeConfigOutputs(
                  std::vector<Workload<LibmcrouterService>*> workloads) {
    return folly::dynamic::object;
  }

 private:
  State state_;
  int index_;
  char buffer[250];
  ContinuousStatistic* cycles_statistic_{nullptr};
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
