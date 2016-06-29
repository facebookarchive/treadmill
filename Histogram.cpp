/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "treadmill/Histogram.h"

#include <glog/logging.h>

using std::vector;

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Constructor of Histogram
 *
 * @param number_of_bins The number of bins in the sample histogram
 * @param min_value The minimum value of the sample histogram
 * @param max_value The maximum value of the sample histogram
 */
Histogram::Histogram(const int number_of_bins, const double min_value,
                     const double max_value)
  : x_values_(vector<double>(number_of_bins)),
    y_values_(vector<double>(number_of_bins)),
    cdf_values_(vector<double>(number_of_bins)) {
  double delta_x = (max_value - min_value) / number_of_bins;
  for (int i = 0; i < number_of_bins; i++) {
    x_values_[i] = (i + 1) * delta_x + min_value;
    y_values_[i] = 0.0;
    cdf_values_[i] = 0.0;
  }
}

/**
 * Add a sample to the histogram
 *
 * @param sample_value The sample value to add
 */
void Histogram::addSample(const double sample_value) {
  int bin_index = Histogram::findClosestBin(x_values_, sample_value);
  bin_index = (bin_index > y_values_.size() - 1) ? (y_values_.size() - 1)
                                                 : bin_index;
  y_values_[bin_index]++;
}

/**
 * Get the X value for a quantile from the sample histogram
 *
 * @param quantile The quantile to search for
 * @return The X value of the quantile in the sample histogram
 */
double Histogram::getQuantile(const double quantile) {
  this->updateCdf();

  int bin_index = Histogram::findClosestBin(cdf_values_, quantile);
  int i = 0;
  for (auto val: cdf_values_) {
    i++;
  }
  double bottom_x = 0.0;
  double bottom_y = 0.0;
  double top_x = cdf_values_[bin_index];
  double top_y = x_values_[bin_index];

  if (bin_index != 0) {
    bottom_x = cdf_values_[bin_index - 1];
    bottom_y = x_values_[bin_index - 1];
  }

  return Histogram::linearInterpolate(bottom_x, top_x, bottom_y, top_y,
                                      quantile);
}

/**
 * Print out the statistic of the histogram
 */
void Histogram::printHistogram() {
  double sample_count = accumulate(y_values_.begin(), y_values_.end(), 0.0);

  LOG(INFO) << "50\% Percentile: " << this->getQuantile(0.50);
  LOG(INFO) << "90\% Percentile: " << this->getQuantile(0.90);
  LOG(INFO) << "95\% Percentile: " << this->getQuantile(0.95);
  LOG(INFO) << "99\% Percentile: " << this->getQuantile(0.99);
}

void Histogram::insertSmallerHistogramSamples(
                  std::unique_ptr<Histogram>& histogram) {
  for (int i = 0; i < histogram->x_values_.size(); i++) {
    int idx = this->findClosestBin(x_values_, histogram->x_values_[i]);
    y_values_[idx] += histogram->y_values_[i];
  }
}

/**
 * Find the first bin that is greater than or equal to searched value
 *
 * @param values The vector of values to search in
 * @param search_value The value to search for
 * @return The index of the closest bin
 */
int Histogram::findClosestBin(const vector<double>& values,
                              const double search_value) {
  auto bin_iter = lower_bound(values.begin(), values.end(), search_value);
  return distance(values.begin(), bin_iter);
}

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
double Histogram::linearInterpolate(const double bottom_x, const double top_x,
                                    const double bottom_y, const double top_y,
                                    const double x_value) {
  return bottom_y +
         (top_y - bottom_y) / (top_x - bottom_x) * (x_value - bottom_x);
}

/**
 * Update the CDF of the sample histogram
 */
void Histogram::updateCdf() {
  double sample_count = accumulate(y_values_.begin(), y_values_.end(), 0.0);
  double current_cdf = 0.0;
  for (int i = 0; i < y_values_.size(); i++) {
    double pdf = y_values_[i] / sample_count;
    current_cdf += pdf;
    cdf_values_[i] = current_cdf;
  }
}

void Histogram::combine(const Histogram& hist) {
  for (int i = 0; i < y_values_.size(); i++) {
    this->y_values_[i] += hist.y_values_[i];
  }
  this->updateCdf();
}

folly::dynamic Histogram::toDynamic() {
  folly::dynamic hist = folly::dynamic::object;
  for (int i = 0; i < y_values_.size(); i++) {
    hist[folly::to<std::string>(x_values_[i])] = y_values_[i];
  }
  return hist;
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
