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
#include "treadmill/services/sleep/SleepService.h"

#include "treadmill/Connection.h"
#include "treadmill/Util.h"
#include "treadmill/StatisticsManager.h"

#include "treadmill/services/sleep/gen-cpp2/Sleep.h"
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>

DECLARE_string(hostname);
DECLARE_int32(port);

namespace facebook {
namespace windtunnel {
namespace treadmill {

template<>
class Connection<SleepService> {
 public:
  Connection<SleepService>(folly::EventBase& event_base) {
    std::string host = nsLookUp(FLAGS_hostname);
    std::shared_ptr<apache::thrift::async::TAsyncSocket> socket(
        apache::thrift::async::TAsyncSocket::newSocket(&event_base,
                                                       host,
                                                       FLAGS_port));
    std::unique_ptr<
      apache::thrift::HeaderClientChannel,
      folly::DelayedDestruction::Destructor> channel(
          new apache::thrift::HeaderClientChannel(socket));

    client_ = folly::make_unique<services::sleep::SleepAsyncClient>(
                std::move(channel));
  }

  bool isReady() const { return true; }

  folly::Future<SleepService::Reply>
  sendRequest(std::unique_ptr<typename SleepService::Request> request) {
    auto f = client_->future_goSleep(request->sleep_time()).then(
      [](folly::Try<int64_t>&& t) mutable {
        StatisticsManager::get().getContinuousStat("SleepTime")
          .addSample(t.value());
        return SleepReply(t.value());
      }
    );
    return f;
  }

 private:
  std::unique_ptr<services::sleep::SleepAsyncClient> client_;
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
