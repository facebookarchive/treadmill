/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <gtest/gtest.h>

#include "RandomEngine.h"

#include <vector>
#include <thread>
#include <numeric>
#include <functional>
#include <cmath>

#include <folly/Memory.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

namespace {

using facebook::windtunnel::treadmill::RandomEngine;
using facebook::windtunnel::treadmill::ThreadSafeRandomEngine;

void checkCorrelation (std::function<double(double,double)> prng) {
  FLAGS_random_seed = 0;

  // test 10 threads
  const int num_threads = 10;
  // test 100000 numbers in [0,100]
  const int total_number = 100000;
  const double range = 100;
  std::vector<double> numbers[num_threads];
  double dev[num_threads];
  std::unique_ptr<std::thread> threads[num_threads];

  // generate numbers
  for (int i = 0; i < num_threads; i++) {
    threads[i] = folly::make_unique<std::thread>(
        [&numbers, &prng, i, total_number, range] {
          for (int j = 0; j < total_number; j++) {
            double x = prng(0, range);
            numbers[i].push_back(x);
          }
        });
  }
  // calculates deviates
  for (int i = 0; i < num_threads; i++) {
    threads[i]->join();
    double avg = std::accumulate(numbers[i].begin(), numbers[i].end(), 0);
    avg /= total_number;
    for (auto& x : numbers[i]) {
      x -= avg;
      dev[i] += x * x;
    }
    dev[i] = sqrt(dev[i]);
  }
  // check correlation
  for (int i = 0; i < num_threads; i++) {
    for (int j = 0; j < i; j++) {
      double corr = 0;
      for (int k = 0; k < total_number; k++) {
        corr += numbers[i][k] * numbers[j][k];
      }
      corr /= dev[i] * dev[j];
      ASSERT_NEAR(0, corr, 0.05);
    }
  }
}

TEST(StatisticTest, Correlation) {
  checkCorrelation(
      static_cast<double (*)(double, double)>(RandomEngine::getDouble));
}

TEST(StatisticTest, CrossThreadCorrelation) {
  checkCorrelation(ThreadSafeRandomEngine::getDouble);
}

} // namespace


}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
