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

#include <folly/Memory.h>

#include "treadmill/Util.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

Scheduler::Scheduler(uint32_t rps, uint32_t number_of_workers,
                     uint32_t logging_threshold)
    : logging_threshold_(logging_threshold), rps_(rps),
      logged_(number_of_workers, 1), queues_(number_of_workers) {
}

Scheduler::~Scheduler() {
}

void Scheduler::run() {
  running_.store(true, std::memory_order_relaxed);
  thread_ = folly::make_unique<std::thread>([this] { this->loop(); });
}

void Scheduler::stopAndJoin() {
  running_.store(false);
  thread_->join();
}

folly::NotificationQueue<int>& Scheduler::getWorkerQueue(uint32_t id) {
  return queues_[id];
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

/**
 * Responsible for generating requests events.
 * Requests are randomly spaced (intervals are drawn from an
 * exponential distribution) to achieve the target throughput rate.
 * Events would be put into notification queues, which would be selected in
 * round-robin fashion.
 */
void Scheduler::loop() {
  int64_t interval_ns = 1.0/rps_ * k_ns_per_s;
  int64_t a = 0, b = 0, budget = randomExponentialInterval(interval_ns);
  while (running_) {
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
    queues_[next_].putMessage(0);
    if (queues_[next_].size() > logging_threshold_ * logged_[next_]) {
      LOG(INFO) << "Notification queue for worker " << next_
                << " is overloaded by factor of " << logged_[next_];
      logged_[next_] *= 2;
    }
    ++next_;
    if (next_ == queues_.size()) {
      next_ = 0;
    }
  }
  /* Shut down all workers */
  for (int i = 0; i < queues_.size(); ++i) {
    queues_[i].putMessage(-1);
  }
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
