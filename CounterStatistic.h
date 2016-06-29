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

#include <string>
#include <unordered_map>
#include <vector>

#include <folly/dynamic.h>

#include "treadmill/Statistic.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

class CounterStatistic : public Statistic {
 public:
  explicit CounterStatistic(const std::string& name)
      : Statistic(name) {}

  CounterStatistic(const CounterStatistic& other)
      : Statistic(other.name_) {
    count_ = other.count_;
    subkey_count_ = other.subkey_count_;
  }

  virtual std::unique_ptr<Statistic> clone() const override {
    return std::unique_ptr<Statistic>(new CounterStatistic(*this));
  }

  virtual void printStatistic() const override;

  virtual folly::dynamic toDynamic() const override;

  virtual std::unordered_map<std::string, int64_t> getCounters() const override;

  virtual void combine(const Statistic& stat) override;

  void increase(size_t n = 1, const std::string& subkey = "") {
    count_ += n;
    if (!subkey.empty()) {
      subkey_count_[subkey] += n;
    }
  }

  size_t getCount() const {
    return count_;
  }

  size_t getCount(const std::string& subkey) const {
    auto it = subkey_count_.find(subkey);
    return (it == subkey_count_.end()) ? 0 : it->second;
  }

 private:
  size_t count_ = 0;
  std::unordered_map<std::string, size_t> subkey_count_;

};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
