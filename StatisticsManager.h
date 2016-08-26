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

#include "treadmill/ContinuousStatistic.h"
#include "treadmill/CounterStatistic.h"
#include "treadmill/Statistic.h"

DECLARE_int32(latency_calibration_samples);
DECLARE_int32(latency_warmup_samples);

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
  StatisticsManager() {}

  virtual ~StatisticsManager() {}

  void print() const;

  static StatisticsManager& get();
  static StatisticsManager getCombined();
  static void printAll();
  static std::string toJson();
  static std::map<std::string, int64_t> exportAllCounters();

  ContinuousStatistic& getContinuousStat(const std::string& name);
  CounterStatistic& getCounterStat(const std::string& name);

  StatisticsManager(StatisticsManager const&);
  void operator=(StatisticsManager const&);

  using StatMapType =
    std::unordered_map<std::string, std::unique_ptr<Statistic>>;

  const StatMapType& getStatMap() const { return stat_map_; }

 private:
  void combine(const StatisticsManager& other);

  StatMapType stat_map_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
