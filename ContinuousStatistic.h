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

#include <vector>

#include <folly/dynamic.h>

#include "treadmill/Histogram.h"
#include "treadmill/Statistic.h"

DECLARE_int32(default_calibration_samples);
DECLARE_int32(default_warmup_samples);

namespace facebook {
namespace windtunnel {
namespace treadmill {

const int kNumberOfBins = 1024;

const int kExceptionalValues = 1000;

/**
 * Uses a similar methodology to:
 * http://web.eecs.umich.edu/~twenisch/papers/ispass12.pdf
 *
 */
class ContinuousStatistic : public Statistic {
 public:
  ContinuousStatistic(const std::string& name,
            int nWarmupSamples,
            int nCalibrationSamples) :
      Statistic(name),
      histogram_(),
      nWarmupSamples_(nWarmupSamples),
      warmupSamples_(0),
      nCalibrationSamples_(nCalibrationSamples),
      s0_(0),
      s1_(0.0),
      s2_(0.0),
      a_(0.0),
      q_(0.0),
      minSet_(false),
      maxSet_(false),
      min_(0),
      max_(0) {}

  ContinuousStatistic(const ContinuousStatistic& s)
    : Statistic(s.name_),
      nWarmupSamples_(s.nWarmupSamples_),
      warmupSamples_(s.warmupSamples_),
      nCalibrationSamples_(s.nCalibrationSamples_),
      s0_(s.s0_),
      s1_(s.s1_),
      s2_(s.s2_),
      a_(s.a_),
      q_(s.q_),
      minSet_(s.minSet_),
      maxSet_(s.maxSet_),
      min_(s.min_),
      max_(s.max_),
      exceptional_index_(s.exceptional_index_) {
    if (s.histogram_ != nullptr) {
      histogram_.reset(new Histogram(*s.histogram_));
    }
    for (int i = 0; i < exceptional_index_; i++) {
      exceptional_values_[i] = s.exceptional_values_[i];
    }
  }

  explicit ContinuousStatistic(const std::string& name) :
    ContinuousStatistic(name,
                        FLAGS_default_warmup_samples,
                        FLAGS_default_calibration_samples) {}

  std::unique_ptr<Statistic> clone() const override {
    return std::unique_ptr<Statistic>(new ContinuousStatistic(*this));
  }

  /**
   * Add a sample to the statistic
   *
   * @param value
   */
  void addSample(double latency);

  double getAverage() const;

  double getStdDev() const;

  double getCV() const;

  /**
   * Estimate a quantile
   *
   * @param quantile
   */
  double getQuantile(double quantile);

  /**
   * Print out all the statistic
   */
  void printStatistic() const override;

  folly::dynamic toDynamic() const override;

  std::unordered_map<std::string, int64_t> getCounters() const override;

  void combine(const Statistic& stat) override;

 private:
  void rebinHistogram(double target_max_value = -1.0);

  void setHistogramBins();

  double meanConfidence() const;

  double quantileConfidence(double quantile) const;

  std::unique_ptr<Histogram> histogram_;
  int nWarmupSamples_;
  int warmupSamples_;
  std::vector<double> calibrationSamples_;
  int nCalibrationSamples_;
  int s0_;
  double s1_;
  double s2_;
  double a_;
  double q_;
  bool minSet_;
  bool maxSet_;
  double min_;
  double max_;
  double exceptional_values_[kExceptionalValues];
  int exceptional_index_{0};
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
