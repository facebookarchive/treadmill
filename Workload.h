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

template <class Workload>
class WorkloadBase {
};

template <class Service>
class Workload {
  /**
   * Workloads are implemented as template specializations that derive from
   * WorkloadBase using themselves as template parameter and also implement:
   *
   * Workload(folly::dynamic config)
   * reset() - to reset their internal state. (Used via Scheduler::resume to
   *           re-synchronize A/B sides after pausing.)
   * getNextRequest() - to get one request from the workload.
   */
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
