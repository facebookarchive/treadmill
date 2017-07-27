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

#include <folly/SharedMutex.h>

#include "common/fb303/cpp/FacebookBase2.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

class TreadmillFB303 : public facebook::fb303::FacebookBase2 {
 public:
  using fb_status = facebook::fb303::cpp2::fb_status;

  explicit TreadmillFB303();

  ~TreadmillFB303() override;

  void setStatus(fb_status status);
  fb_status getStatus() override;

  void getStatusDetails(std::string& _return) override;
  int64_t aliveSince() override;
  void getCounters(std::map<std::string, int64_t>& _return) override;

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
