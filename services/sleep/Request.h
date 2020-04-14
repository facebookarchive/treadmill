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

#include <folly/futures/Future.h>

#include "treadmill/Request.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

class SleepReply {
 public:
  explicit SleepReply(int64_t sleep_time) : sleep_time_(sleep_time) {}

  int64_t sleep_time() {
    return sleep_time_;
  }

 private:
  double sleep_time_;
};

class SleepRequest : public Request {
 public:
  enum Operation { SLEEP };

  SleepRequest(Operation type, int64_t sleep_time)
      : type_(type), sleep_time_(sleep_time) {}

  virtual ~SleepRequest() {}

  Operation which() {
    return type_;
  }

  int64_t sleep_time() {
    return sleep_time_;
  }

  std::string getRequestType() {
    return "SleepRequest";
  }

 private:
  Operation type_;
  int64_t sleep_time_;
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
