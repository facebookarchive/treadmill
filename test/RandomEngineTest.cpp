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

#include "treadmill/RandomEngine.h"

#include <vector>
#include <thread>
#include <numeric>
#include <functional>
#include <cmath>


namespace facebook {
namespace windtunnel {
namespace treadmill {

namespace {

using facebook::windtunnel::treadmill::RandomEngine;
using facebook::windtunnel::treadmill::ThreadSafeRandomEngine;

void checkCorrelation (std::function<double(double,double)> prng) {
  // test 10 threads
  const int kNumThreads = 10;
  // test 100000 numbers in [0,100]
  const int kNumSamples = 100000;
  const double kRange = 100;
  std::vector<double> numbers[kNumThreads];
  double dev[kNumThreads];
  std::unique_ptr<std::thread> threads[kNumThreads];

  // generate numbers
  for (int i = 0; i < kNumThreads; i++) {
    threads[i] = std::make_unique<std::thread>(
        [&numbers, &prng, i, kNumSamples, kRange] {
          for (int j = 0; j < kNumSamples; j++) {
            double x = prng(0, kRange);
            numbers[i].push_back(x);
          }
        });
  }
  // calculates deviates
  for (int i = 0; i < kNumThreads; i++) {
    threads[i]->join();
    double avg = std::accumulate(numbers[i].begin(), numbers[i].end(), 0);
    avg /= kNumSamples;
    LOG(INFO) << "mean of " << i << " is " << avg;
    dev[i] = 0;
    for (auto& x : numbers[i]) {
      ASSERT_NEAR(x, kRange / 2, kRange / 2 + 0.001);
      x -= avg;
      dev[i] += x * x;
    }
    LOG(INFO) << "variance of " << i << " is " << dev[i];
    dev[i] = sqrt(dev[i]);
  }
  // check correlation
  for (int i = 0; i < kNumThreads; i++) {
    for (int j = 0; j < i; j++) {
      double corr = 0;
      for (int k = 0; k < kNumSamples; k++) {
        corr += numbers[i][k] * numbers[j][k];
      }
      LOG(INFO) << "corvariance between " << i << " and " << j << " is "
                << corr;
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
  FLAGS_treadmill_random_seed = 0;
  return RUN_ALL_TESTS();
}
