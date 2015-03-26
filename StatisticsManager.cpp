/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "StatisticsManager.h"

#include <glog/logging.h>

#include <folly/dynamic.h>
#include <folly/json.h>
#include <folly/ThreadLocal.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

class StatsTag;
static folly::ThreadLocal<StatisticsManager, StatsTag> localManager;

StatisticsManager& StatisticsManager::get() {
  return *localManager;
}

void StatisticsManager::printAll() const {
  StatisticsManager combined = this->getCombined();
  combined.print();
}

void StatisticsManager::print() const {
  LOG(INFO) << "Statistics:";
  LOG(INFO) << "";
  for (auto& stat: stat_map_) {
    LOG(INFO) << stat.second.getName();
    stat.second.printStatistic();
  }
}

void StatisticsManager::combine(StatisticsManager& other) {
  for (auto& pair: other.getStats()) {
    auto key = pair.first;
    Statistic& stat = pair.second;
    Statistic& thisStat = this->getStat(key);
    thisStat.combine(stat);
  }
}

Statistic& StatisticsManager::getStat(const std::string& name) {
  if (stat_map_.find(name) == stat_map_.end()) {
    if (name == REQUEST_LATENCY) {
      // More warmup and calibration samples for request latency
      stat_map_.emplace(name, Statistic(name, 1000, 1000));
    } else {
      stat_map_.emplace(name, Statistic(name));
    }
  }
  return stat_map_[name];
}

StatisticsManager StatisticsManager::getCombined() const {
  StatisticsManager combined;
  for (auto& manager : localManager.accessAllThreads()) {
    combined.combine(manager);
  }
  return combined;
}


std::string StatisticsManager::toJson() const {
  StatisticsManager combined = this->getCombined();
  folly::dynamic map = folly::dynamic::object;
  for (auto& pair: combined.getStats()) {
    map[pair.first] = pair.second.toDynamic();
  }
  folly::json::serialization_opts opts;
  opts.allow_nan_inf = true;
  opts.allow_non_string_keys = true;
  return folly::json::serialize(map, opts).toStdString();
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
