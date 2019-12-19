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
#include "treadmill/if/gen-cpp2/TreadmillService.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

class Scheduler;

class TreadmillFB303 : public facebook::fb303::FacebookBase2,
                       public ::treadmill::TreadmillServiceSvIf {
 public:
  using fb_status = facebook::fb303::cpp2::fb_status;

  explicit TreadmillFB303(Scheduler& scheduler);

  ~TreadmillFB303() override;

  void setStatus(fb_status status);
  fb_status getStatus() override;

  void getStatusDetails(std::string& _return) override;
  int64_t aliveSince() override;
  void getCounters(std::map<std::string, int64_t>& _return) override;

  bool pause() override;
  bool resume() override;
  folly::Future<std::unique_ptr<::treadmill::ResumeResponse>> future_resume2(
      std::unique_ptr<::treadmill::ResumeRequest> req) override;
  void setRps(int32_t rps) override;
  void setMaxOutstanding(int32_t max_outstanding) override;
  folly::Future<std::unique_ptr<::treadmill::RateResponse>> future_getRate() override;

  folly::Future<std::unique_ptr<std::string>> future_getConfiguration(
      std::unique_ptr<std::string> key) override;
  void setConfiguration(std::unique_ptr<std::string> key,
      std::unique_ptr<std::string> value) override;
  uint32_t getConfigurationValue(const std::string &key, uint32_t defaultValue);
  std::unique_ptr<std::string> getConfigurationValue(const std::string &key,
      const std::string &defaultValue);
  void clearConfiguration() override;
  bool configurationEmpty() const;

  void watchdogUpdate();
  bool watchdogTimeoutCheck(bool raise=true);

  static void make_fb303(
      std::shared_ptr<std::thread>& server_thread,
      int server_port,
      Scheduler& scheduler);

 private:
  fb_status status_;
  const int64_t aliveSince_;
  folly::SharedMutex mutex_;
  Scheduler& scheduler_;
  std::unique_ptr<std::map<std::string, std::string>> configuration_;
  uint32_t watchdogDurationSec_;
  time_t lastHeartbeat_;
};

extern std::shared_ptr<TreadmillFB303> getGlobalTreadmillFB303();

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
