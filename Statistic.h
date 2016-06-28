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

#include <folly/dynamic.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

class Statistic {
 public:
  explicit Statistic(const std::string& name)
    : name_(name) {}

  virtual ~Statistic() {}

  virtual std::unique_ptr<Statistic> clone() const = 0;

  virtual void printStatistic() const = 0;

  virtual folly::dynamic toDynamic() const = 0;

  // Get fb303 counters.
  virtual std::unordered_map<std::string, int64_t> getCounters() const = 0;

  virtual void combine(const Statistic& stat) = 0;

  std::string getName() const {
    return name_;
  }

 protected:
  std::string name_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
