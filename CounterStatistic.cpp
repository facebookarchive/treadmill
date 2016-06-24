/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "treadmill/CounterStatistic.h"

#include "treadmill/Util.h"

#include <cmath>
#include <mutex>
#include <unordered_map>

#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Print out all the statistic
 */
void CounterStatistic::printStatistic() const {
  LOG(INFO) << "Count: " << count_;
  for (const auto& p : subkey_count_) {
    LOG(INFO) << "Count[" << p.first << "]: " << p.second;
  }
}

folly::dynamic CounterStatistic::toDynamic() const {
  folly::dynamic map = folly::dynamic::object;
  map["count"] = this->count_;
  for (const auto& p : subkey_count_) {
    map[p.first] = p.second;
  }
  return map;
}

std::unordered_map<std::string, int64_t> CounterStatistic::getCounters() const {
  std::unordered_map<std::string, int64_t> m;
  m[name_] = count_;
  for (const auto& p : subkey_count_) {
    m[name_ + '.' + p.first] = p.second;
  }
  return m;
}

void CounterStatistic::combine(const Statistic& stat0) {
  const CounterStatistic& stat =
    dynamic_cast<const CounterStatistic&>(stat0);

  count_ += stat.count_;
  for (const auto& p : stat.subkey_count_) {
    subkey_count_[p.first] += p.second;
  }
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
