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
public:
  void setPhase(const std::string& phase) {
    phase_ = phase;
  }
  const std::string getPhase() const {
    return phase_;
  }

protected:
  std::string phase_;
};

template <class Service>
class Workload {
  /**
   * Workloads are implemented as template specializations that derive from
   * WorkloadBase using themselves as template parameter. Example:
   *   class Workload<MyService> : BaseWorkload<Workload<MyService>> {...};
   *
   * Any specialization of Workload must implement the following functions:
   *  explicit Workload<Service>(folly::dynamic config)
   *  void reset() - to reset their internal state. (Used via Scheduler::resume
   *                 to re-synchronize A/B sides after pausing.)
   *  std::tuple<
   *              std::unique_ptr<HhvmHttpReplayService::Request>,
   *              folly::Promise<HhvmHttpReplayService::Reply>,
   *              folly::Future<HhvmHttpReplayService::Reply>
   *            > getNextRequest() - to get one request from the workload.
   *  folly::dynamic makeConfigOutputs(
   *      std::vector<Workload<HhvmHttpReplayService>*>)
   */
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
