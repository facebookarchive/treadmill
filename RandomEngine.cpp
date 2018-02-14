/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "treadmill/RandomEngine.h"

#include <thread>
#include <sys/time.h>

#include <folly/Likely.h>

DEFINE_uint64(treadmill_random_seed, ULLONG_MAX, "seed for random engines");

using std::mt19937_64;
using std::uniform_real_distribution;
using std::uniform_int_distribution;

namespace facebook {
namespace windtunnel {
namespace treadmill {

// Seed the random engine
mt19937_64 RandomEngine::random_engine_(
    FLAGS_treadmill_random_seed == ULLONG_MAX ? time(nullptr) : FLAGS_treadmill_random_seed);
// Generate a uniform distribution
uniform_real_distribution<double>
  RandomEngine::uniform_real_distribution_(0.0, 1.0);
uniform_int_distribution<uint64_t>
  RandomEngine::uniform_int_distribution_(0, ULLONG_MAX);

// Empty thread-local random engine
folly::ThreadLocalPtr<std::mt19937_64> ThreadSafeRandomEngine::random_engine_;

double RandomEngine::getDouble() {
  return uniform_real_distribution_(random_engine_);
}

double RandomEngine::getDouble(double min, double max) {
  uniform_real_distribution<double> dist(min, max);
  return dist(random_engine_);
}

uint64_t RandomEngine::getInteger() {
  return uniform_int_distribution_(random_engine_);
}

uint64_t RandomEngine::getInteger(uint64_t min, uint64_t max) {
  uniform_int_distribution<uint64_t> dist(min, max);
  return dist(random_engine_);
}

mt19937_64& ThreadSafeRandomEngine::get() {
  mt19937_64 *engine = random_engine_.get();
  if (UNLIKELY(engine == nullptr)) {
    std::hash<std::thread::id> hasher;
    uint64_t seed = hasher(std::this_thread::get_id()) +
                    (FLAGS_treadmill_random_seed == ULLONG_MAX
                         ? time(nullptr)
                         : FLAGS_treadmill_random_seed);
    engine = new mt19937_64(seed);
    random_engine_.reset(engine);
  }
  return *engine;
}

double ThreadSafeRandomEngine::getDouble(double min, double max) {
  uniform_real_distribution<double> dist(min, max);
  return dist(get());
}

uint64_t ThreadSafeRandomEngine::getInteger(uint64_t min, uint64_t max) {
  uniform_int_distribution<uint64_t> dist(min, max);
  return dist(get());
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
