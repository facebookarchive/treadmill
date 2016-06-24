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

namespace facebook {
namespace windtunnel {
namespace treadmill {

class HistogramInput {
 public:
  HistogramInput(int number_of_bins, int min_value, int max_value) :
    number_of_bins_(number_of_bins),
    min_value_(min_value),
    max_value_(max_value) {}

  HistogramInput() :
    HistogramInput(0, 0, 0) {}

  int number_of_bins_;
  int min_value_;
  int max_value_;
};

// Class for histogram of the sampling data
class Histogram {
 public:
  /**
   * Constructor of Histogram
   *
   * @param number_of_bins The number of bins in the sample histogram
   * @param min_value The minimum value of the sample histogram
   * @param max_value The maximum value of the sample histogram
   */
  Histogram(const int number_of_bins,
            const double min_value,
            const double max_value);

  explicit Histogram(const HistogramInput& input) :
    Histogram(input.number_of_bins_, input.min_value_, input.max_value_) {}

  /**
   * Add a sample to the histogram
   *
   * @param sample_value The sample value to add
   */
  void addSample(const double sample_value);
  /**
   * Get the X value for a quantile from the sample histogram
   *
   * @param quantile The quantile to search for
   * @return The X value of the quantile in the sample histogram
   */
  double getQuantile(const double quantile);
  /**
   * Print out the statistic of the histogram
   */
  void printHistogram();

  void combine(const Histogram& hist);

  double getMinBin() const {
    return x_values_.front();
  }

  double getMaxBin() const {
    return x_values_.back();
  }

  void insertSmallerHistogramSamples(std::unique_ptr<Histogram>& histogram);

  /**
   * Returns a dynamic map representing the histogram
   */
  folly::dynamic toDynamic();

 private:
  /**
   * Find the bin that is closest to the searched value
   *
   * @param values The vector of values to search in
   * @param search_value The value to search for
   * @return The index of the closest bin
   */
  static int findClosestBin(const std::vector<double>& values,
                            const double search_value);
  /**
   * Simple linear interpolation
   *
   * @param bottom_x Lower bound of X value
   * @param top_x Upper bound of X value
   * @param bottom_y Corresponding Y value of the lower bound X value
   * @param top_y Corresponding Y value of the upper bound Y value
   * @param x_value The X value to interpolate
   * @return The interpolated Y value
   */
  static double linearInterpolate(const double bottom_x, const double top_x,
                                  const double bottom_y, const double top_y,
                                  const double x_value);
  /**
   * Update the CDF of the sample histogram
   */
  void updateCdf();

  // The X values of the bins in the sample histogram
  std::vector<double> x_values_;
  // The Y values (counts) of the bins in the sample histogram
  std::vector<double> y_values_;
  // The CDF of the sample histogram
  std::vector<double> cdf_values_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
