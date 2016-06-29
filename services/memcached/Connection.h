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

#include <folly/MoveWrapper.h>
#include <folly/fibers/EventBaseLoopController.h>
#include <folly/fibers/FiberManager.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <mcrouter/lib/network/AsyncMcClient.h>
#include <mcrouter/lib/network/gen-cpp2/mc_caret_protocol_types.h>
#include <mcrouter/lib/network/TypedThriftMessage.h>

#include "treadmill/Connection.h"
#include "treadmill/StatisticsManager.h"
#include "treadmill/Util.h"
#include "treadmill/services/memcached/MemcachedService.h"

DECLARE_string(hostname);
DECLARE_int32(port);

using facebook::memcache::AsyncMcClient;
using facebook::memcache::ConnectionOptions;
using facebook::memcache::cpp2::McDeleteRequest;
using facebook::memcache::cpp2::McGetRequest;
using facebook::memcache::cpp2::McSetRequest;
using facebook::memcache::McOperation;
using facebook::memcache::TypedThriftRequest;
using folly::fibers::EventBaseLoopController;
using folly::fibers::FiberManager;

namespace facebook {
namespace windtunnel {
namespace treadmill {

template <>
class Connection<MemcachedService> {
 public:
  explicit Connection<MemcachedService>(folly::EventBase& event_base) {
    std::string host = nsLookUp(FLAGS_hostname);
    ConnectionOptions opts(host, FLAGS_port, mc_ascii_protocol);
    client_ = folly::make_unique<AsyncMcClient>(event_base, opts);
    auto loopController = folly::make_unique<EventBaseLoopController>();
    loopController->attachEventBase(event_base);
    fm_ = folly::make_unique<FiberManager>(std::move(loopController));
  }

  bool isReady() const { return true; }

  folly::Future<MemcachedService::Reply>
  sendRequest(std::unique_ptr<typename MemcachedService::Request> request) {
    folly::MoveWrapper<folly::Promise<MemcachedService::Reply> > p;
    auto f = p->getFuture();

    if (request->which() == MemcachedRequest::GET) {
      auto req =
        std::make_shared<TypedThriftRequest<McGetRequest>>(request->key());
      fm_->addTask([this, req, p] () mutable {
        client_->sendSync(*req, std::chrono::milliseconds::zero());
        p->setValue(MemcachedService::Reply());
      });
    } else if (request->which() == MemcachedRequest::SET) {
      auto req =
        std::make_shared<TypedThriftRequest<McSetRequest>>(request->key());
      req->setValue(request->value());
      fm_->addTask([this, req, p] () mutable {
        client_->sendSync(*req, std::chrono::milliseconds::zero());
        p->setValue(MemcachedService::Reply());
      });
    } else {
      auto req =
        std::make_shared<TypedThriftRequest<McDeleteRequest>>(request->key());
      fm_->addTask([this, req, p] () mutable {
        client_->sendSync(*req, std::chrono::milliseconds::zero());
        p->setValue(MemcachedService::Reply());
      });
    }
    return f;
  }

 private:
  std::unique_ptr<AsyncMcClient> client_;
  std::unique_ptr<FiberManager> fm_;
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
