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

#include <folly/Memory.h>

DECLARE_string(counter_name);
DECLARE_int32(counter_threshold);

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Specializations of this template should implement:
 *
 * bool isReady();
 *
 * folly::Future<Service::Reply>
 * sendRequest(std::unique_ptr<typename Service::Request>&& request);
 *
 * */
template <class Service>
class Connection {
public:
/**
 * Sample implementation below shows how to wait for a specific counter
 * value to cross a threshold
 * e.g. usage: --wait_for_target_ready --counter_threshold 10
 * --counter_name thrift.accepted_connections.count
 * */

  bool isReady() const {
    if(!FLAGS_counter_name.empty()) {
      int64_t value = client_->sync_getCounter(FLAGS_counter_name);
      if(value < FLAGS_counter_threshold) {
        LOG(INFO) << "Threshold: "
          << FLAGS_counter_threshold << " Counter: "
          << FLAGS_counter_name << " Value: " << value;
        return false;
      }
    }
    return true;
  }
private:
  std::unique_ptr<typename Service::Client> client_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
