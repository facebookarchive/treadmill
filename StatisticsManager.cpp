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

#include <folly/Format.h>
#include <folly/Memory.h>
#include <folly/Singleton.h>
#include <folly/dynamic.h>
#include <folly/json.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

namespace {
struct PrivateTag {};

static folly::Singleton<StatisticsManager, PrivateTag> managerSingle;
const auto kQuantiles = std::array<double, 13>{{0.01,
                                                0.05,
                                                0.10,
                                                0.15,
                                                0.20,
                                                0.50,
                                                0.80,
                                                0.85,
                                                0.90,
                                                0.95,
                                                0.99,
                                                0.999,
                                                1.0}};

} // namespace
/* static */ std::shared_ptr<StatisticsManager> StatisticsManager::get() {
  return managerSingle.try_get();
}

void StatisticsManager::print() const {
  LOG(INFO) << "Statistics:";
  LOG(INFO) << "";
  count_map_.withWLock([](auto& m) {
    for (auto& cp : m) {
      LOG(INFO) << cp.first;
      cp.second->printStatistic();
    }
  });

  histo_map_.withWLock([](auto& m) {
    for (auto& cp : m) {
      // Unlike the counter there's no printStatistic for our histograms. So we
      // have to do that here.
      LOG(INFO) << cp.first;
      cp.second->flush();
      auto est = cp.second->estimateQuantiles(kQuantiles);

      LOG(INFO) << "Count: " << folly::sformat("{:.0f}", est.count);
      LOG(INFO) << "Avg: " << est.sum / std::max(1.0, est.count);
      for (const auto& q : est.quantiles) {
        LOG(INFO) << folly::sformat("P{:.0f}: {:.2f}", q.first * 100, q.second);
      }
    }
  });
}

std::shared_ptr<StatisticsManager::Histogram>
StatisticsManager::getContinuousStat(const std::string& name) {
  return histo_map_.withWLock([&](auto& m) {
    auto it = m.find(name);

    if (it != m.end()) {
      return it->second;
    } else {
      // We don't want to construct a counter unless we know the counter isn't
      // there. That does lead to two reads into the map. Oh well.
      auto ptr = std::make_shared<StatisticsManager::Histogram>();
      m.emplace(name, ptr);
      return ptr;
    }
  });
}

std::shared_ptr<StatisticsManager::Counter> StatisticsManager::getCounterStat(
    const std::string& name) {
  return count_map_.withWLock([&](auto& m) {
    // Try and see if the counter is in the map
    auto it = m.find(name);

    if (it != m.end()) {
      return it->second;
    } else {
      // We don't want to construct a counter unless we know the counter isn't
      // there. That does lead to two reads into the map. Oh well.
      auto ptr = std::make_shared<StatisticsManager::Counter>(name);
      m.emplace(name, ptr);
      return ptr;
    }
  });
}

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
