/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "treadmill/Scheduler.h"

#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/Memory.h>

#include "treadmill/Util.h"

DEFINE_bool(wait_for_runner_ready,
            false,
            "If true, wait for a 'resume' message before sending requests.");

namespace facebook {
namespace windtunnel {
namespace treadmill {

Scheduler::Scheduler(uint32_t rps, uint32_t number_of_workers,
                     uint32_t logging_threshold)
    : logging_threshold_(logging_threshold), rps_(rps),
      max_outstanding_requests_(0),
      logged_(number_of_workers, 1), queues_(number_of_workers) {
  state_.store(FLAGS_wait_for_runner_ready ? PAUSED : RUNNING,
               std::memory_order_relaxed);
}

Scheduler::Scheduler(uint32_t rps, uint32_t number_of_workers,
                     uint32_t max_outstanding_requests,
                     uint32_t logging_threshold)
    : logging_threshold_(logging_threshold), rps_(rps),
      max_outstanding_requests_(max_outstanding_requests),
      logged_(number_of_workers, 1), queues_(number_of_workers) {
  state_.store(FLAGS_wait_for_runner_ready ? PAUSED : RUNNING,
               std::memory_order_relaxed);
}

Scheduler::~Scheduler() {
}

folly::Future<folly::Unit> Scheduler::run() {

  if (state_ != RUNNING) {
    LOG(INFO) << "Scheduler is not in the running state. "
              << "Assuming resume will be called in future.";
  }
  thread_ = std::make_unique<std::thread>([this] { this->loop(); });
  return promise_.getFuture();
}

void Scheduler::pause() {
  RunState expected = RUNNING;
  state_.compare_exchange_strong(expected, PAUSED);
}

bool Scheduler::resume() {
  RunState expected = PAUSED;
  state_.compare_exchange_strong(expected, RUNNING);

  // Now return if we are running. It's possible that the scheduler was already
  // running so we don't return the bool from compare_exchange_strong.
  return state_ == RUNNING;
}

bool Scheduler::isRunning() {
  return state_ == RUNNING;
}

void Scheduler::setPhase(const std::string& phase_name) {
  if (FLAGS_wait_for_runner_ready) {
    CHECK_EQ(state_, PAUSED);
  }
  messageAllWorkers(Event(EventType::SET_PHASE, phase_name));
}

int32_t Scheduler::getMaxOutstandingRequests() {
  return max_outstanding_requests_;
}

void Scheduler::setMaxOutstandingRequests(int32_t max_outstanding_requests) {
  max_outstanding_requests_ = max_outstanding_requests;
  messageAllWorkers(Event(EventType::SET_MAX_OUTSTANDING, max_outstanding_requests_));
}

void Scheduler::stop() {
  state_.store(STOPPING);
}

void Scheduler::join() {
  CHECK(state_ == STOPPING);
  thread_->join();
}

folly::NotificationQueue<Event>& Scheduler::getWorkerQueue(uint32_t id) {
  return queues_[id];
}

int32_t Scheduler::getRps() {
  return rps_;
}

void Scheduler::setRps(int32_t rps) {
  rps_ = rps;
}

double Scheduler::randomExponentialInterval(double mean) {
  static std::mt19937* rng = new std::mt19937();
  std::uniform_real_distribution<double> dist(0, 1.0);
  /* Cap the lower end so that we don't return infinity */
  return - log(std::max(dist(*rng), 1e-9)) * mean;
}

void Scheduler::waitNs(int64_t ns) {
  /* We need to have *precise* timing, and it's not achievable with any other
     means like 'nanosleep' or EventBase.
     "pause" instruction would hint processor that this is a spin-loop, it
     will burn as much CPU as possible. The processor will use this hint
     to avoid memory order violation, which greatly improves its performance.
     http://siyobik.info.gf/main/reference/instruction/PAUSE */
  for (auto start = nowNs(); nowNs() - start < ns;) {
    asm volatile("pause");
  }
}

void Scheduler::messageAllWorkers(Event event) {
  for (int i = 0; i < queues_.size(); ++i) {
    queues_[i].putMessage(event);
  }
}

/**
 * Responsible for generating requests events.
 * Requests are randomly spaced (intervals are drawn from an
 * exponential distribution) to achieve the target throughput rate.
 * Events would be put into notification queues, which would be selected in
 * round-robin fashion.
 */
void Scheduler::loop() {
  do {
    messageAllWorkers(Event(EventType::RESET));
    next_ = 0;
    int32_t rps = rps_;
    int64_t interval_ns = 1.0/rps * k_ns_per_s;
    int64_t a = 0, b = 0, budget = randomExponentialInterval(interval_ns);
    while (state_ == RUNNING) {
      b = nowNs();
      if (a) {
        /* Account for time spent sending the message */
        budget -= (b - a);
      }
      waitNs(std::max(budget, 0L));
      a = nowNs();
      /* Decrease the sleep budget by the exact time slept (could have been
         more than the budget value), increase by the next interval */
      budget += randomExponentialInterval(interval_ns) - (a - b);
      queues_[next_].putMessage(Event(EventType::SEND_REQUEST));
      if (queues_[next_].size() > logging_threshold_ * logged_[next_]) {
        LOG(INFO) << "Notification queue for worker " << next_
                  << " is overloaded by factor of " << logged_[next_];
        logged_[next_] *= 2;
      }
      ++next_;
      if (next_ == queues_.size()) {
        next_ = 0;
      }
      if (rps != rps_) {
        rps = rps_;
        interval_ns = 1.0/rps * k_ns_per_s;
      }
    }
    while (state_ == PAUSED) waitNs(1000);
  } while (state_ != STOPPING);
  messageAllWorkers(Event(EventType::STOP));
  promise_.setValue(folly::Unit());
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
