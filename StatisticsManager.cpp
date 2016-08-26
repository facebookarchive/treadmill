/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "treadmill/StatisticsManager.h"

#include <glog/logging.h>

#include <folly/Memory.h>
#include <folly/dynamic.h>
#include <folly/json.h>
#include <folly/ThreadLocal.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

class StatsTag;
static folly::ThreadLocal<StatisticsManager, StatsTag> localManager;

/* static */ StatisticsManager& StatisticsManager::get() {
  return *localManager;
}

/* static */ void StatisticsManager::printAll() {
  StatisticsManager combined = getCombined();
  combined.print();
}

void StatisticsManager::print() const {
  LOG(INFO) << "Statistics:";
  LOG(INFO) << "";
  for (auto& stat: stat_map_) {
    LOG(INFO) << stat.second->getName();
    stat.second->printStatistic();
  }
}

void StatisticsManager::combine(const StatisticsManager& other) {
  for (const auto& pair: other.stat_map_) {
    const auto& key = pair.first;
    const Statistic& stat = *pair.second;
    auto it = stat_map_.find(key);
    if (it == stat_map_.end()) {
      stat_map_[key] = stat.clone();
    } else {
      it->second->combine(stat);
    }
  }
}

ContinuousStatistic& StatisticsManager::getContinuousStat(
    const std::string& name) {
  auto it = stat_map_.find(name);
  if (it == stat_map_.end()) {
    if (name == REQUEST_LATENCY) {
      // More warmup and calibration samples for request latency
      it = stat_map_.emplace(name, folly::make_unique<ContinuousStatistic>(
        name, FLAGS_latency_warmup_samples, FLAGS_latency_calibration_samples)
      ).first;
    } else {
      it = stat_map_.emplace(name, folly::make_unique<ContinuousStatistic>(
        name)).first;
    }
  }
  return dynamic_cast<ContinuousStatistic&>(*it->second);
}

CounterStatistic& StatisticsManager::getCounterStat(
    const std::string& name) {
  auto it = stat_map_.find(name);
  if (it == stat_map_.end()) {
    it = stat_map_.emplace(name, folly::make_unique<CounterStatistic>(
      name)).first;
  }
  return dynamic_cast<CounterStatistic&>(*it->second);
}

/* static */ StatisticsManager StatisticsManager::getCombined() {
  StatisticsManager combined;
  for (const auto& manager : localManager.accessAllThreads()) {
    combined.combine(manager);
  }
  return combined;
}


/* static */ std::string StatisticsManager::toJson() {
  StatisticsManager combined = getCombined();
  folly::dynamic map = folly::dynamic::object;
  for (auto& pair: combined.stat_map_) {
    map[pair.first] = pair.second->toDynamic();
  }
  folly::json::serialization_opts opts;
  opts.allow_nan_inf = true;
  opts.allow_non_string_keys = true;
  return folly::json::serialize(map, opts);
}

/* static */
std::map<std::string, int64_t> StatisticsManager::exportAllCounters() {
  StatisticsManager combined = getCombined();
  std::map<std::string, int64_t> m;
  for (const auto& p1: combined.stat_map_) {
    const auto m2 = p1.second->getCounters();
    m.insert(m2.begin(), m2.end());
  }
  return m;
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
