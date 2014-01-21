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

#include "Distribution.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Get a quantile from the distribution by index
 *
 * @param index The index of the queried quantile
 * @return The quantile value
 */
double Distribution::getQuantileByIndex(int index) {
  return cdf_values_[index];
}

/**
 * Constructor for ConstantDistribution
 *
 * @param constant_value The value of the constant in the distribution
 */
ConstantDistribution::ConstantDistribution(const double constant_value) {
  for (int i = 0; i < kNumberOfValues; i++) {
    cdf_values_[i] = constant_value;
  }
}

/**
 * Constructor for UniformDistribution
 *
 * @param min_value The minimal value of the uniform distribution
 * @param max_value The maximal value of the uniform distribution
 */
UniformDistribution::UniformDistribution(const double min_value,
                                         const double max_value) {
  double delta_value = (max_value - min_value) / kNumberOfValues;
  for (int i = 0; i < kNumberOfValues; i++) {
    cdf_values_[i] = min_value + delta_value * i;
  }
}

/**
 * Constructor for ExponentialDistribution
 *
 * @param mean_value The mean value of the exponential distribution
 */
ExponentialDistribution::ExponentialDistribution(const double mean_value) {
  for (int i = 0; i< kNumberOfValues; i++) {
    cdf_values_[i] = -log(1 - (double) i / (double) kNumberOfValues)
                     * mean_value;
  }
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
