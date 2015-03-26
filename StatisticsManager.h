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

#include <unordered_map>

#include "Statistic.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

// Statistics names are kept here
const std::string REQUEST_LATENCY = "request_latency";
const std::string THROUGHPUT = "throughput";
const std::string OUTSTANDING_REQUESTS = "outstanding_requests";

class StatisticsManager {
 public:
  StatisticsManager() {}

  virtual ~StatisticsManager() {}

  void printAll() const;

  void print() const;

  std::string toJson() const;

  void combine(StatisticsManager& other);

  static StatisticsManager& get();

  Statistic& getStat(const std::string& name);

  std::unordered_map<std::string, Statistic>& getStats() {
    return stat_map_;
  }

 private:
  StatisticsManager(StatisticsManager const&);
  void operator=(StatisticsManager const&);
  StatisticsManager getCombined() const;

  std::unordered_map<std::string, Statistic> stat_map_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
