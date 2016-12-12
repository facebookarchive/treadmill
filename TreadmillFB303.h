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

#include <folly/SharedMutex.h>
#include <folly/Singleton.h>

#include "common/fb303/if/gen-cpp2/FacebookService.h"
#include "treadmill/StatisticsManager.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

// TODO(14039001): commonize this so other treadmills can use it
class TreadmillFB303 : public facebook::fb303::cpp2::FacebookServiceSvIf {
 public:
  using fb_status = facebook::fb303::cpp2::fb_status;

  explicit TreadmillFB303()
    : status_(fb_status::STARTING),
      aliveSince_(time(nullptr)) {}

  ~TreadmillFB303() override {}

  void setStatus(fb_status status) {
    folly::SharedMutex::WriteHolder(mutex_);
    status_ = status;
  }

  fb_status getStatus() override {
    folly::SharedMutex::ReadHolder(mutex_);
    return status_;
  }

  void getStatusDetails(std::string& _return) override {
    _return = fb303::cpp2::_fb_status_VALUES_TO_NAMES.at(getStatus());
  }

  int64_t aliveSince() override {
    folly::SharedMutex::ReadHolder(mutex_);
    return aliveSince_;
  }

  void getCounters(std::map<std::string, int64_t>& _return) override {
    _return = StatisticsManager::exportAllCounters();
  }

  static void make_fb303(
      std::shared_ptr<std::thread>& server_thread,
      int server_port);

 private:
  fb_status status_;
  const int64_t aliveSince_;
  folly::SharedMutex mutex_;
};

extern std::shared_ptr<TreadmillFB303> getGlobalTreadmillFB303();

}
}
}
