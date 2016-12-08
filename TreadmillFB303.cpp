/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "TreadmillFB303.h"

#include <memory>

#include <folly/Singleton.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

namespace {
folly::Singleton<TreadmillFB303> instance;
}

std::shared_ptr<TreadmillFB303> getGlobalTreadmillFB303() {
  return instance.try_get();
}

}
}
}
