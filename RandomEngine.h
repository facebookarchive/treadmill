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

#include <random>

#include <folly/ThreadLocal.h>

DECLARE_uint64(treadmill_random_seed);

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Shared struct for Mersene Twister 19937 generator (64 bit)
 *
 * The struct produces a shared random number stream that all threads can
 * access.
 * The struct might not be thread-safe or have high locking contention depending
 * on the underlying STL implementation.
 * If you want a thread-safe PRNG for each thread, use ThreadSafeRandomEngine.
 */
struct RandomEngine {
 public:
  /**
   * Return a random number ranging in [0.0, 1.0] in double
   *
   * @return A random number ranging in [0.0, 1.0] in double
   */
  static double getDouble();

  /**
   * Return a random number ranging in [min, max] in double
   *
   * @return A random number ranging in [min, max] in double
   */
  static double getDouble(double min, double max);

  /**
   * Return a random number ranging in [0, 2^64-1] in uint64_t
   *
   * @return A random number ranging in [0, 2^64-1] in uint64_t
   */
  static uint64_t getInteger();

  /**
   * Return a random number ranging in [min, max] in uint64_t
   *
   * @return A random number ranging in [min, max] in uint64_t
   */
  static uint64_t getInteger(uint64_t min, uint64_t max);

 private:
  // The Mersene Twister 19937 random engine (64 bit)
  static std::mt19937_64 random_engine_;
  // A uniform distribution for real numbers
  static std::uniform_real_distribution<double> uniform_real_distribution_;
  // A uniform distribution for integer numbers
  static std::uniform_int_distribution<uint64_t> uniform_int_distribution_;
};

/**
 * Thread-local struct for Mersene Twister 19937 generator (64 bit)
 *
 * This struct produces a private random number stream for current thread.
 * Should perform better than the shared engine.
 * Different threads will have different seeds derived from the global seed and
 * their thread ids.
 * The correlation between streams of all threads might not be 0.
 * Don't use if you have strong requirement on randomness.
 */
struct ThreadSafeRandomEngine {
 public:
  /**
   * Return a random number ranging in [min, max] in double
   *
   * @return A random number ranging in [min, max] in double
   */
  static double getDouble(double min, double max);

  /**
   * Return a random number ranging in [min, max] in uint64_t
   *
   * @return A random number ranging in [min, max] in uint64_t
   */
  static uint64_t getInteger(uint64_t min, uint64_t max);

 private:
  /**
   * Return the underlying random engine
   *
   * @return The underlying random engine
   */
  static std::mt19937_64& get();

  // The Mersene Twister 19937 random engine (64 bit)
  static folly::ThreadLocalPtr<std::mt19937_64> random_engine_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
