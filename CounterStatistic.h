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

#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

#include <folly/AtomicUnorderedMap.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

class CounterStatistic {
 public:
  explicit CounterStatistic(const std::string& /* name */) {}

  void increase(int64_t n, const std::string& subkey) {
    count_ += n;
    if (!subkey.empty()) {
      subkey_count_
          .findOrConstruct(
              subkey,
              [](void* raw) { return new (raw) std::atomic<int64_t>{0}; })
          .first->second.data += n;
    }
  }

  int64_t getCount() const {
    return count_;
  }

  int64_t getCount(const std::string& subkey) {
    return subkey_count_
        .findOrConstruct(
            subkey,
            [=](void* raw) { return new (raw) std::atomic<int64_t>{0}; })
        .first->second.data;
  }

  void printStatistic() const;

 private:
  int64_t count_ = 0;
  folly::AtomicUnorderedInsertMap<std::string, folly::MutableAtom<int64_t>>
      subkey_count_ = folly::
          AtomicUnorderedInsertMap<std::string, folly::MutableAtom<int64_t>>{
              10 * 1024};
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
