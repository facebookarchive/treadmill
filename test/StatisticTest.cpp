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

#include "treadmill/ContinuousStatistic.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

namespace {

TEST(StatisticTest, SimpleAverage) {
  ContinuousStatistic s("test", 0, 0);
  for (int i = 0; i < 100; i++) {
    s.addSample(i + 1);
  }
  ASSERT_NEAR(50.5, s.getAverage(), .1);
}

TEST(StatisticTest, RebinTest) {
  ContinuousStatistic s_0("stat_0", 0, 0);
  ContinuousStatistic s_1("stat_1", 0, 0);
  for (int i = 0; i < 100; i++) {
    s_0.addSample(i + 1);
    s_1.addSample(i + 1);
  }
  s_0.combine(s_1);
  ASSERT_NEAR(s_0.getQuantile(0.5), 50, 1.0);
  ASSERT_NEAR(s_0.getQuantile(0.95), 95, 1.0);
  ASSERT_NEAR(s_0.getQuantile(0.99), 99, 1.0);
}

} // namespace


}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
