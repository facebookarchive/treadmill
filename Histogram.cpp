/*
* Copyright (c) 2013, Facebook, Inc.
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*   * Neither the name Facebook nor the names of its contributors may be used to
*     endorse or promote products derived from this software without specific
*     prior written permission.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <glog/logging.h>

#include "Histogram.h"

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
    x_values_[i] = (i + 1) * delta_x;
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
  y_values_[bin_index] += 1;
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

  LOG(INFO) << "\tTotal Count: " << sample_count;
  LOG(INFO) << "\t50\% Percentile Latency: " << this->getQuantile(0.50);
  LOG(INFO) << "\t90\% Percentile Latency: " << this->getQuantile(0.90);
  LOG(INFO) << "\t95\% Percentile Latency: " << this->getQuantile(0.95);
  LOG(INFO) << "\t99\% Percentile Latency: " << this->getQuantile(0.99);
}

/**
 * Find the first bin that is greater than or equal to searched value
 *
 * @param values The vector of values to search in
 * @param search_value The value to search for
 * @return The index of the closest bin
 */
int Histogram::findClosestBin(const vector<double> values,
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

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
