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

#include <vector>

#include "treadmill/services/memcached/MemcachedService.h"

#include "treadmill/Workload.h"

DECLARE_int64(number_of_keys);

using folly::Future;
using folly::Promise;

namespace facebook {
namespace windtunnel {
namespace treadmill {

template <>
class Workload<MemcachedService>
    : public WorkloadBase<Workload<MemcachedService>> {
 public:
  enum State {
    WARMUP,
    GET
  };

  Workload<MemcachedService>(folly::dynamic /*config*/)
      : state_(State::WARMUP), index_(0) {}

  void reset() {
    index_ = 0;
  }

  std::tuple<std::unique_ptr<MemcachedService::Request>,
             Promise<MemcachedService::Reply>,
             Future<MemcachedService::Reply>>
  getNextRequest() {
    if (index_ == FLAGS_number_of_keys) {
      index_ = 0;
    }

    std::string key = std::to_string(index_);

    std::unique_ptr<MemcachedService::Request> request;
    if (state_ == State::WARMUP) {
      request = std::make_unique<MemcachedRequest>(MemcachedRequest::SET,
                                                     std::move(key));
      request->setValue(std::to_string(index_));
      if (index_ == FLAGS_number_of_keys - 1) {
        LOG(INFO) << "WARMUP complete";
        state_ = State::GET;
      }
    } else if (state_ == State::GET) {
      request = std::make_unique<MemcachedRequest>(MemcachedRequest::GET,
                                                     std::move(key));
    }
    Promise<MemcachedService::Reply> p;
    auto f = p.getFuture();
    ++index_;
    return std::make_tuple(std::move(request), std::move(p), std::move(f));
  }

  folly::dynamic makeConfigOutputs(
      std::vector<Workload<MemcachedService>*> /*workloads*/) {
    return folly::dynamic::object;
  }

 private:
  State state_;
  int index_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
