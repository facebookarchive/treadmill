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

#include "boost/variant.hpp"
#include "memcache/client/CacheClientString.h"

DEFINE_string(libmcrouter_flavor,
              "perf",
              "The flavor of libmcrouter to use.");

DEFINE_string(libmcrouter_keys_prefix,
              "windtunnel.treadmill.libmcrouter.testKey",
              "Will be used as a prefix for keys.");

namespace facebook {
namespace windtunnel {
namespace treadmill {

class LibmcrouterService {
 public:
  typedef boost::variant<facebook::memcache::CacheClientString::UpdateRequests,
                         facebook::memcache::CacheClientString::GetRequests>
  Request;

  typedef boost::variant<facebook::memcache::CacheClientString::UpdateResults,
                         facebook::memcache::CacheClientString::GetResults>
  Reply;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
