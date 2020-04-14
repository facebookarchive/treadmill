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
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/NotificationQueue.h>
#include <folly/system/ThreadName.h>

#include "treadmill/Connection.h"
#include "treadmill/Event.h"
#include "treadmill/StatisticsManager.h"
#include "treadmill/Util.h"
#include "treadmill/Workload.h"

DECLARE_bool(wait_for_target_ready);
DECLARE_string(counter_name);
DECLARE_int32(counter_threshold);

namespace facebook {
namespace windtunnel {
namespace treadmill {

constexpr folly::StringPiece kOutstandingRequestsCounter =
    "outstanding_requests";

template <class Service>
class Worker : private folly::NotificationQueue<Event>::Consumer {
 public:
  Worker(
      int worker_id,
      folly::NotificationQueue<Event>& queue,
      int number_of_workers,
      int number_of_connections,
      int max_outstanding_requests,
      const folly::dynamic& config,
      int cpu_affinity,
      std::function<void()> terminate_early_fn)
      : worker_id_(worker_id),
        number_of_workers_(number_of_workers),
        number_of_connections_(number_of_connections),
        max_outstanding_requests_(max_outstanding_requests),
        workload_(config),
        cpu_affinity_(cpu_affinity),
        queue_(queue),
        terminate_early_fn_(terminate_early_fn) {
    for (int i = 0; i < number_of_connections_; i++) {
      connections_.push_back(
          std::make_unique<Connection<Service>>(event_base_));
    }

    setWorkerCounter(kOutstandingRequestsCounter, 0);
  }

  ~Worker() override {}

  void run() {
    // If countername is specified then make sure wait_for_target was also true
    if (!FLAGS_counter_name.empty() &&
        (!FLAGS_wait_for_target_ready || FLAGS_counter_threshold < 0)) {
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
    sender_thread_ =
        std::make_unique<std::thread>([this] { this->senderLoop(); });
  }

  void stop() {
    running_.store(false);
    auto stopper = [this]() { event_base_.terminateLoopSoon(); };
    event_base_.runInEventBaseThread(stopper);
    LOG(INFO) << "Worker " << worker_id_ << " terminating";
  }

  void join() {
    sender_thread_->join();
  }

  bool hasMoreWork() {
    return running_ || outstanding_requests_ > 0;
  }

  folly::dynamic makeConfigOutputs(std::vector<Worker*> worker_refs) {
    std::vector<Workload<Service>*> workload_refs;
    for (auto worker : worker_refs) {
      workload_refs.push_back(&(worker->workload_));
    }
    return workload_.makeConfigOutputs(workload_refs);
  }

 private:
  void setWorkerCounter(folly::StringPiece key, int64_t value) {
    std::string fullKey = folly::sformat("worker.{}.{}", worker_id_, key);

    auto sd = facebook::stats::ServiceData::get();
    sd->setCounter(fullKey, value);
  }

  void setMaxOutstanding(int32_t max_outstanding_requests) {
    max_outstanding_requests_ = max_outstanding_requests;
  }

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
    auto manager = StatisticsManager::get();
    outstanding_statistic_ = manager->getContinuousStat(OUTSTANDING_REQUESTS);
    throughput_statistic_ = manager->getContinuousStat(THROUGHPUT);
    latency_statistic_ = manager->getContinuousStat(REQUEST_LATENCY);
    exceptions_statistic_ = manager->getCounterStat(EXCEPTIONS);
    uncaught_exceptions_statistic_ =
        manager->getCounterStat(UNCAUGHT_EXCEPTIONS);
    last_throughput_time_ = nowNs();

    startConsuming(&event_base_, &queue_);
    event_base_.loopForever();
  }

  void messageAvailable(Event&& event) noexcept override {
    if (event.getEventType() == EventType::STOP || !running_) {
      LOG(INFO) << "Stopping Worker because "
                << ((event.getEventType() == EventType::STOP)
                        ? "Event Type = Stop"
                        : "running_ = false but got a message.");
      stopConsuming();
      // To avoid potential race condition
      running_.store(false);
    } else if (event.getEventType() == EventType::RESET) {
      LOG(INFO) << "Got EventType::RESET";
      workload_.reset();
    } else if (event.getEventType() == EventType::SEND_REQUEST) {
      sendRequest();
    } else if (event.getEventType() == EventType::SET_MAX_OUTSTANDING) {
      auto extraData = event.getExtraData();
      if (!extraData.isInt()) {
        LOG(ERROR) << "SET_MAX_OUTSTANDING event not an int: " << extraData;
      } else {
        LOG(INFO) << "Got EventType::SET_MAX_OUTSTANDING = "
                  << extraData.asInt();
        setMaxOutstanding(extraData.asInt());
      }
    } else if (event.getEventType() == EventType::SET_PHASE) {
      auto extraData = event.getExtraData();
      if (!extraData.isString()) {
        LOG(ERROR) << "SET_PHASE event got invalid extra data: " << extraData;
      } else {
        LOG(INFO) << "Got EventType::SET_PHASE = " << extraData.asString();
        workload_.setPhase(extraData.asString());
      }
    } else {
      LOG(ERROR) << "Got unhandled event: " << int(event.getEventType());
    }
  }

  void sendRequest() {
    if (outstanding_requests_ < max_outstanding_requests_ && running_) {
      auto request_tuple = workload_.getNextRequest();
      if (std::get<0>(request_tuple) == nullptr) {
        LOG(INFO) << "terminating";
        running_.store(false);
        terminate_early_fn_();
        return;
      }
      auto pw = folly::makeMoveWrapper(std::move(std::get<1>(request_tuple)));
      ++outstanding_requests_;
      auto conn_idx = conn_idx_;
      conn_idx_ = (conn_idx_ + 1) % number_of_connections_;
      auto send_time = nowNs();

      auto reply =
          connections_[conn_idx]
              ->sendRequest(std::move(std::get<0>(request_tuple)))
              .thenTry([send_time, this, pw](
                           folly::Try<typename Service::Reply>&& t) mutable {
                auto recv_time = nowNs();
                if (running_) {
                  // If the worker is not in running state, latency stat have
                  // already been released
                  latency_statistic_->addValue(
                      (recv_time - send_time) / 1000.0);
                }
                n_throughput_requests_++;
                if (t.hasException()) {
                  n_exceptions_by_type_
                      [t.exception().class_name().toStdString()]++;
                  LOG(INFO) << t.exception().what();
                  pw->setException(t.exception());
                }
                if (t.hasValue()) {
                  pw->setValue(std::move(t.value()));
                }

                --outstanding_requests_;
                this->setWorkerCounter(
                    kOutstandingRequestsCounter, outstanding_requests_);
              });
      auto& f = std::get<2>(request_tuple);
      std::move(f).thenError([this](folly::exception_wrapper ew) {
        n_uncaught_exceptions_by_type_[ew.class_name().toStdString()]++;
        return folly::makeFuture<
            typename std::remove_reference<decltype(f)>::type::value_type>(ew);
      });
    }

    // Estimate throughput and outstanding requests
    auto t = nowNs();
    double throughput_delta = double(t - last_throughput_time_) / k_ns_per_s;
    if (throughput_delta >= 0.1) {
      double throughput =
          n_throughput_requests_ / throughput_delta * number_of_workers_;
      throughput_statistic_->addValue(throughput);
      n_throughput_requests_ = 0;
      last_throughput_time_ = t;
      double outstanding = outstanding_requests_ * number_of_workers_;
      outstanding_statistic_->addValue(outstanding);
    }

    for (auto p : n_exceptions_by_type_) {
      exceptions_statistic_->increase(p.second, p.first);
    }
    n_exceptions_by_type_.clear();

    for (auto p : n_uncaught_exceptions_by_type_) {
      uncaught_exceptions_statistic_->increase(p.second, p.first);
    }
    n_uncaught_exceptions_by_type_.clear();

    setWorkerCounter(kOutstandingRequestsCounter, outstanding_requests_);
  }

  const int worker_id_;
  std::vector<std::unique_ptr<Connection<Service>>> connections_;
  folly::EventBase event_base_;
  std::atomic<bool> running_;
  int number_of_workers_;
  int number_of_connections_;
  int32_t max_outstanding_requests_;
  Workload<Service> workload_;
  int cpu_affinity_;
  int64_t last_throughput_time_{0};
  std::atomic<int64_t> n_throughput_requests_{0};
  std::unordered_map<std::string, size_t> n_exceptions_by_type_;
  std::unordered_map<std::string, size_t> n_uncaught_exceptions_by_type_;

  folly::NotificationQueue<Event>& queue_;
  std::unique_ptr<std::thread> sender_thread_;
  size_t conn_idx_{0};
  std::atomic<int64_t> outstanding_requests_{0};
  std::shared_ptr<StatisticsManager::Histogram> latency_statistic_{nullptr};
  std::shared_ptr<StatisticsManager::Histogram> outstanding_statistic_{nullptr};
  std::shared_ptr<StatisticsManager::Histogram> throughput_statistic_{nullptr};
  std::shared_ptr<StatisticsManager::Counter> exceptions_statistic_{nullptr};
  std::shared_ptr<StatisticsManager::Counter> uncaught_exceptions_statistic_{
      nullptr};
  std::function<void()> terminate_early_fn_;
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
