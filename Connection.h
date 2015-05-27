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
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
