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

#include "Statistic.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Constructor for Statistic
 */
Statistic::Statistic() {

}

/**
 * Add a statistic histogram for certain operation type
 *
 * @param operation_type The operation type to look up
 */
void Statistic::addStatistic(OperationType operation_type) {
  Histogram histogram (kNumberOfBins, kLowerBoundLatency,
                       kUpperBoundLatency);
  histograms_.insert(make_pair(operation_type, histogram));
}

/**
 * Add a sample to statistic
 *
 * @param latency The latency of the sampled request
 * @param operation_type The operation type of the sampled request
 */
void Statistic::addSample(double latency, OperationType operation_type) {
  // Add sample to all operation histogram
  if (histograms_.find(ALL_OPERATION) != histograms_.end()) {
    histograms_.find(ALL_OPERATION)->second.addSample(latency);
  }
  // Add sample one corresponding operation histogram
  if (histograms_.find(operation_type) != histograms_.end() ) {
    histograms_.find(operation_type)->second.addSample(latency);
  }
}

/**
 * Get the quantile for particular operation type
 *
 * @param quantile The quantile to search for
 * @param operation_type The operation type querying for,
 *        ALL_OPERATION for all types of operations
 */
double Statistic::getQuantile(double quantile, OperationType operation_type) {
  if (histograms_.find(operation_type) != histograms_.end()) {
    return histograms_.find(operation_type)->second.getQuantile(quantile);
  } else {
    throw NonExistingHistogramException();
  }
}

/**
 * Reset all the statistics to initial stat
 */
void Statistic::reset() {
  histograms_.clear();
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
