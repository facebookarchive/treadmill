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

#include <sched.h>

#include <memory>
#include <thread>

#include <glog/logging.h>

#include <folly/Memory.h>
#include <folly/MoveWrapper.h>
#include <folly/String.h>
#include <folly/ThreadName.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/NotificationQueue.h>

#include "treadmill/Connection.h"
#include "treadmill/StatisticsManager.h"
#include "treadmill/Util.h"
#include "treadmill/Workload.h"

DEFINE_bool(wait_for_target_ready,
            false,
            "If true, wait until the target is ready.");

DECLARE_string(counter_name);
DECLARE_int32(counter_threshold);

namespace facebook {
namespace windtunnel {
namespace treadmill {

template <class Service>
class Worker : private folly::NotificationQueue<int>::Consumer {
 public:
  Worker(folly::NotificationQueue<int>& queue,
         int number_of_workers,
         int number_of_connections,
         int max_outstanding_requests,
         const folly::dynamic& config,
         int cpu_affinity) :
      number_of_workers_(number_of_workers),
      number_of_connections_(number_of_connections),
      max_outstanding_requests_(max_outstanding_requests),
      workload_(config),
      cpu_affinity_(cpu_affinity),
      queue_(queue) {
    for (int i = 0; i < number_of_connections_; i++) {
      connections_.push_back(
                    folly::make_unique<Connection<Service>>(event_base_));
    }
  }

  ~Worker() {}

  void run() {
    //If countername is specified then make sure wait_for_target was also true
    if (!FLAGS_counter_name.empty() && (!FLAGS_wait_for_target_ready ||
        FLAGS_counter_threshold<0)) {
      LOG(FATAL) << "--counter_name " << FLAGS_counter_name
        << " specified without --wait_for_target_ready"
        << " or valid --counter_threshold value";
    }

    if (FLAGS_wait_for_target_ready) {
      for (auto& conn : connections_) {
        while (!conn->isReady()) {
          LOG(INFO) << "Target not ready";
          /* sleep override */ sleep(1);
        }
      }
      LOG(INFO) << "Target is ready";
    }

    running_.store(true, std::memory_order_relaxed);
    sender_thread_ = folly::make_unique<std::thread>(
      [this] { this->senderLoop(); });
  }

  void stop() {
    running_.store(false);
  }

  void join() {
    sender_thread_->join();
  }

  folly::dynamic makeConfigOutputs(std::vector<Worker*> worker_refs) {
    std::vector<Workload<Service>*> workload_refs;
    for (auto worker: worker_refs) {
      workload_refs.push_back(&(worker->workload_));
    }
    return workload_.makeConfigOutputs(workload_refs);
  }

 private:

  /**
   * Sender loop listens to the request queue and network events.
   * It will only send up to the outstanding requests limit.
   */
  void senderLoop() {
    folly::setThreadName("treadmill-wrkr");
    if (cpu_affinity_ != -1) {
      cpu_set_t mask;
      CPU_ZERO(&mask);
      CPU_SET(cpu_affinity_, &mask);
      if (sched_setaffinity(0, sizeof(cpu_set_t), &mask)) {
        LOG(ERROR) << "Failed to set CPU affinity";
      }
    }
    throughput_statistic_ = &StatisticsManager::get()
      .getContinuousStat(THROUGHPUT);
    outstanding_statistic_ = &StatisticsManager::get()
      .getContinuousStat(OUTSTANDING_REQUESTS);
    latency_statistic_ = &StatisticsManager::get()
      .getContinuousStat(REQUEST_LATENCY);
    exceptions_statistic_ = &StatisticsManager::get()
      .getCounterStat(EXCEPTIONS);
    uncaught_exceptions_statistic_ = &StatisticsManager::get()
      .getCounterStat(UNCAUGHT_EXCEPTIONS);
    last_throughput_time_ = nowNs();

    startConsuming(&event_base_, &queue_);
    event_base_.loopForever();
  }

  void messageAvailable(int&& message) override {
    if (message == -1 || !running_) {
      stopConsuming();
      if (to_send_ == 0 && outstanding_requests_ == 0) {
        event_base_.terminateLoopSoon();
      } else {
        // To avoid potential race condition
        running_.store(false);
      }
      return;
    }

    ++to_send_;
    pumpRequests();
  }

  void pumpRequests() {
    while (to_send_ &&
           outstanding_requests_ < max_outstanding_requests_ &&
           running_) {

      auto request_tuple = workload_.getNextRequest();
      auto pw = folly::makeMoveWrapper(std::move(std::get<1>(request_tuple)));
      ++outstanding_requests_;
      --to_send_;
      auto conn_idx = conn_idx_;
      conn_idx_ = (conn_idx_ + 1) % number_of_connections_;
      auto send_time = nowNs();

      auto reply = connections_[conn_idx]->sendRequest(
        std::move(std::get<0>(request_tuple))).then(
          [send_time, this, pw] (
            folly::Try<typename Service::Reply>&& t) mutable {

            auto recv_time = nowNs();
            latency_statistic_->addSample((recv_time - send_time)/1000.0);
            n_throughput_requests_++;
            if (t.hasException()) {
              n_exceptions_by_type_[t.exception().class_name().toStdString()]++;
              LOG(INFO) << t.exception().what();
              pw->setException(t.exception());
            }
            if (t.hasValue()) {
              pw->setValue(t.value());
            }

            --outstanding_requests_;

            if (!running_ && outstanding_requests_ == 0) {
              event_base_.terminateLoopSoon();
            } else {
              pumpRequests();
            }
          }
        );
      auto& f = std::get<2>(request_tuple);
      f.onError([this](folly::exception_wrapper ew) {
        n_uncaught_exceptions_by_type_[ew.class_name().toStdString()]++;
        return folly::makeFuture<
            typename std::remove_reference<decltype(f)>::type::value_type>(ew);
      });
    }

    // Estimate throughput and outstanding requests
    auto t = nowNs();
    double throughput_delta = double(t - last_throughput_time_)/k_ns_per_s;
    if (throughput_delta >= 0.1) {
      double throughput = n_throughput_requests_ / throughput_delta *
                          number_of_workers_;
      throughput_statistic_->addSample(throughput);
      n_throughput_requests_ = 0;
      last_throughput_time_ = t;
      double outstanding = outstanding_requests_ * number_of_workers_;
      outstanding_statistic_->addSample(outstanding);
    }

    for (auto p : n_exceptions_by_type_) {
      exceptions_statistic_->increase(p.second, p.first);
    }
    n_exceptions_by_type_.clear();

    for (auto p : n_uncaught_exceptions_by_type_) {
      uncaught_exceptions_statistic_->increase(p.second, p.first);
    }
    n_uncaught_exceptions_by_type_.clear();
  }

  std::vector<std::unique_ptr<Connection<Service>>> connections_;
  folly::EventBase event_base_;
  std::atomic<bool> running_;
  int number_of_workers_;
  int number_of_connections_;
  size_t max_outstanding_requests_;
  Workload<Service> workload_;
  int cpu_affinity_;
  int64_t last_throughput_time_{0};
  size_t n_throughput_requests_{0};
  std::unordered_map<std::string, size_t> n_exceptions_by_type_;
  std::unordered_map<std::string, size_t> n_uncaught_exceptions_by_type_;

  folly::NotificationQueue<int>& queue_;
  std::unique_ptr<std::thread> sender_thread_;
  size_t conn_idx_{0};
  size_t outstanding_requests_{0};
  size_t to_send_{0};
  ContinuousStatistic* latency_statistic_{nullptr};
  ContinuousStatistic* throughput_statistic_{nullptr};
  ContinuousStatistic* outstanding_statistic_{nullptr};
  CounterStatistic* exceptions_statistic_{nullptr};
  CounterStatistic* uncaught_exceptions_statistic_{nullptr};
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
