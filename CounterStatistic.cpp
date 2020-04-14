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
  std::for_each(
      subkey_count_.cbegin(), subkey_count_.cend(), [](const auto& p) {
        LOG(INFO) << "Count[" << p.first << "]: " << p.second.data;
      });
}

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
