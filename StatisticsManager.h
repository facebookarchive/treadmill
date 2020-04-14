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
#include <unordered_map>

#include <folly/Synchronized.h>
#include <folly/stats/QuantileEstimator.h>

#include "treadmill/CounterStatistic.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

// Statistics names are kept here
const std::string REQUEST_LATENCY = "request_latency";
const std::string THROUGHPUT = "throughput";
const std::string OUTSTANDING_REQUESTS = "outstanding_requests";
const std::string EXCEPTIONS = "exceptions";
const std::string UNCAUGHT_EXCEPTIONS = "uncaught_exceptions";

class StatisticsManager {
 public:
  using Histogram = folly::SimpleQuantileEstimator<>;
  using Counter = CounterStatistic;
  using HistoMapType =
      std::unordered_map<std::string, std::shared_ptr<Histogram>>;
  using CounterMapType =
      std::unordered_map<std::string, std::shared_ptr<Counter>>;

  StatisticsManager() {}
  virtual ~StatisticsManager() {}

  void print() const;
  std::string toJson();

  static std::shared_ptr<StatisticsManager> get();
  std::shared_ptr<Histogram> getContinuousStat(const std::string& name);
  std::shared_ptr<Counter> getCounterStat(const std::string& name);

  StatisticsManager(StatisticsManager const&);
  void operator=(StatisticsManager const&);

  folly::Synchronized<HistoMapType> histo_map_;
  folly::Synchronized<CounterMapType> count_map_;
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
