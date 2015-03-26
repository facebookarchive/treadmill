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

#include "Histogram.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

const int kNumberOfBins = 1024;

const int kCalibrationSamples = 10;

const int kWarmupSamples = 10;

const int kExceptionalValues = 1000;

/**
 * Uses a similar methodology to:
 * http://web.eecs.umich.edu/~twenisch/papers/ispass12.pdf
 *
 */
class Statistic {
 public:
  Statistic() : Statistic("") {}

  Statistic(const std::string name,
            int nWarmupSamples,
            int nCalibrationSamples) :
      name_(name),
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

  Statistic(const Statistic& s)
    : name_(s.name_),
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

  explicit Statistic(const std::string name) :
    Statistic(name, kWarmupSamples, kCalibrationSamples) {}
  /**
   * Add a statistic histogram for certain operation type
   *
   * @param operation_type The operation type to look up
   */
  void addStatistic(const std::string& operation_type);

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
  void printStatistic() const;

  folly::dynamic toDynamic() const;

  void combine(const Statistic& stat);

  std::string getName() const {
    return name_;
  }

 private:
  void rebinHistogram(double target_max_value = -1.0);

  void setHistogramBins();

  double meanConfidence() const;

  double quantileConfidence(double quantile) const;

  std::string name_;
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
