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

#include "memcache/client/Constants.h"
#include "memcache/client/MC.h"

#include "Connection.h"

#include "services/libmcrouter/LibmcrouterService.h"

DECLARE_string(libmcrouter_flavor);

namespace facebook {
namespace windtunnel {
namespace treadmill {

template <>
class Connection<LibmcrouterService> {
 public:
  explicit Connection(folly::EventBase& eventBase) {
    facebook::memcache::MC::Options extraOptions;
    if (FLAGS_libmcrouter_flavor == "no-network") {
      extraOptions["no_network"] = "1";
      extraOptions["num_proxies"] = "1";
    }
    auto connection = facebook::memcache::MC::getInstance(determineFlavor())
      .createInternalConnection(extraOptions);
    cc_ = folly::make_unique<facebook::memcache::CacheClientString>(
      std::move(connection), eventBase);
  }

  bool isReady() const { return true; }

  folly::Future<LibmcrouterService::Reply>
  sendRequest(std::unique_ptr<LibmcrouterService::Request> request) {
    return std::move(*boost::apply_visitor(RequestTypeVisitor(*cc_), *request));
  }

 private:
  std::unique_ptr<facebook::memcache::CacheClientString> cc_;

  facebook::memcache::constants::McrouterFlavors determineFlavor() {
    if (FLAGS_libmcrouter_flavor == "web") {
      return facebook::memcache::constants::McrouterFlavors::WEB;
    } else if (FLAGS_libmcrouter_flavor == "no-network" ||
               FLAGS_libmcrouter_flavor == "perf") {
      return facebook::memcache::constants::McrouterFlavors::PERF;
    } else {
      LOG(WARNING) << "Unsupported flavor, using 'perf'!";
      return facebook::memcache::constants::McrouterFlavors::PERF;
    }
  }

  class RequestTypeVisitor : public boost::static_visitor<
      folly::MoveWrapper<folly::Future<LibmcrouterService::Reply>>> {
   public:
    explicit RequestTypeVisitor(facebook::memcache::CacheClientString& cc)
      : cc_(cc) {
    }

    result_type operator() (
        facebook::memcache::CacheClientString::UpdateRequests& requests) const {
      return folly::makeMoveWrapper(cc_.multiSetFuture(requests)
        .then([](folly::Try<
                  facebook::memcache::CacheClientString::UpdateResults>&& t) {
                return LibmcrouterService::Reply(std::move(t.value()));
              }));
    }

    result_type operator() (
        facebook::memcache::CacheClientString::GetRequests& requests) const {
      return folly::makeMoveWrapper(cc_.multiGetFuture(requests)
        .then([](folly::Try<
                  facebook::memcache::CacheClientString::GetResults>&& t) {
                return LibmcrouterService::Reply(std::move(t.value()));
              }));
    }
   private:
    const facebook::memcache::CacheClientString& cc_;
  };
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
