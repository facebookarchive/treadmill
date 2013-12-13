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

#pragma once

#include <memory>
#include <numeric>
#include <vector>

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::accumulate;
using std::distance;
using std::lower_bound;
using std::unique_ptr;
using std::vector;

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
    Histogram(const int number_of_bins, const double min_value,
              const double max_value);

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

  private:
    /**
     * Find the bin that is closest to the searched value
     *
     * @param values The vector of values to search in
     * @param search_value The value to search for
     * @return The index of the closest bin
     */
    static int findClosestBin(const vector<double> values, const double search_value);
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
    vector<double> x_values_;
    // The Y values (counts) of the bins in the sample histogram
    vector<double> y_values_;
    // The CDF of the sample histogram
    vector<double> cdf_values_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
