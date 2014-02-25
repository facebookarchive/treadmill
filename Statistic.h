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

#include <exception>
#include <map>
#include <memory>
#include <vector>

#include "Histogram.h"
#include "KeyRecord.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::exception;
using std::make_pair;
using std::map;

// Upper bound for latency
const double kUpperBoundLatency = 1024.0 * 1024.0;
// Lower bound for latency
const double kLowerBoundLatency = 0.0;
// Number of bins
const int kNumberOfBins = 1024 * 1024;

// Class for non-existing histogram excpetion
class NonExistingHistogramException : public exception {
  const char* what () const throw () {
    return "Non-existing histogram exception";
  }
};

// Class for statistic tools
class Statistic {
  public:
    /**
     * Constructor for Statistic
     */
    Statistic();

    /**
     * Add a statistic histogram for certain operation type
     *
     * @param operation_type The operation type to look up
     */
    void addStatistic(OperationType operation_type);
    /**
     * Add a sample to corresponding sample histograms
     *
     * @param latency The latency of the sampled request
     * @param operation_type The operation type of the sampled request
     */
    void addSample(double latency, OperationType operation_type);
    /**
     * Get the quantile for particular operation type
     *
     * @param quantile The quantile to search for
     * @param operation_type The operation type querying for,
     *        ALL_OPERATION for all types of operations
     */
    double getQuantile(double quantile, OperationType operation_type);
    /**
     * Reset all the statistics to initial state
     */
    void reset();
    /**
     * Print out all the statistic
     */
    void printStatistic();

  private:
    // A vector of added samples
    map<OperationType, Histogram> histograms_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
