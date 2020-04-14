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
#include "treadmill/StatisticsManager.h"
#include "treadmill/Util.h"

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include "treadmill/services/sleep/gen-cpp2/Sleep.h"

DECLARE_string(hostname);
DECLARE_int32(port);

namespace facebook {
namespace windtunnel {
namespace treadmill {

template <>
class Connection<SleepService> {
 public:
  Connection<SleepService>(folly::EventBase& event_base)
      : histo_(StatisticsManager::get()->getContinuousStat("SleepTime")) {
    std::string host = nsLookUp(FLAGS_hostname);
    std::shared_ptr<folly::AsyncSocket> socket(
        folly::AsyncSocket::newSocket(&event_base, host, FLAGS_port));
    std::unique_ptr<
        apache::thrift::HeaderClientChannel,
        folly::DelayedDestruction::Destructor>
        channel(new apache::thrift::HeaderClientChannel(socket));

    client_ =
        std::make_unique<services::sleep::SleepAsyncClient>(std::move(channel));
  }

  bool isReady() const {
    return true;
  }

  folly::Future<SleepService::Reply> sendRequest(
      std::unique_ptr<typename SleepService::Request> request) {
    auto f = client_->future_goSleep(request->sleep_time())
                 .thenTry([histo = histo_](folly::Try<int64_t>&& t) mutable {
                   histo->addValue(t.value());
                   return SleepReply(t.value());
                 });
    return f;
  }

 private:
  std::shared_ptr<StatisticsManager::Histogram> histo_;
  std::unique_ptr<services::sleep::SleepAsyncClient> client_;
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
