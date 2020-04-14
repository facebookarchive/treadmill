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

#include "treadmill/services/sleep/Request.h"

DEFINE_int64(
    sleep_time,
    1000,
    "Requested microseconds to sleep for (default:1000)");

namespace facebook {
namespace windtunnel {
namespace treadmill {

class SleepService {
 public:
  typedef SleepRequest Request;
  typedef SleepReply Reply;
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
