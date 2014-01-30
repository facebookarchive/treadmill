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
#include <string>
#include <vector>

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::exception;
using std::lower_bound;
using std::map;
using std::string;
using std::vector;

// Enumerator for operation types
enum OperationType {
  ALL_OPERATION = 0,
  GET_OPERATION = 1,
  SET_OPERATION = 2,
};

// Constant map of operation types
static map<string, OperationType> kOperationTypeMap = {
  {"get_operation", GET_OPERATION},
  {"set_operation", SET_OPERATION}
};

// Class for out-of-range random value exception
class OutOfRangeRandomValueException : public exception {
  const char* what () const throw () {
    return "Out-of-range random value (should be in [0.0, 1.0)) exception";
  }
};

// Class for illegal distribution function
class IllegalDistributionException : public exception {
  const char* what () const throw () {
    return "Illegal cumulative distribution function";
  }
};

// Class for individual record of a specific key in the workload
class KeyRecord {
  public:
    /**
     * Constructor of KeyRecord
     *
     * @param key The key of the record in string
     * @param key_cdf The cumulative distribution function value of the key
     * @param operation_cdf The cumulative distribution function for
     *                      the operation types
     * @param object_size_cdf The cumulative distribution function for
     *                        the object sizes
     */
    KeyRecord(const string& key, const double key_cdf,
           const map<double, OperationType>& operation_cdf,
           const map<double, int>& object_size_cdf);
    /**
     * Get the key of a record
     *
     * @return The key of a record
     */
    string key();
    /**
     * Get the cumulative distribution function value of a key
     *
     * @return The cumulative distribution function value
     */
    double key_cdf();
    /**
     * Get the cumulative distribution function for the operation types
     *
     * @return The cumulative distribution function for the operation
     *         types
     */
    map<double, OperationType> operation_cdf();
    /**
     * Get the cumulative distribution function for the object sizes
     *
     * @return The cumulative distribution function for the object sizes
     */
    map<double, int> object_size_cdf();
    /**
     * Get a random operation type based on the cumulative distribution
     * function for the operation types
     *
     * @param random_value A random value in [0.0, 1.0)
     * @return An operation type
     */
    OperationType getRandomOperation(const double random_value);
    /**
     * Get a random object size based on the cumulative distribution
     * function for the object sizes
     *
     * @param random_value A random value in [0.0, 1.0)
     * @return An object size
     */
    int getRandomObjectSize(const double random_value);

  private:
    // The key of the workload record in string
    string key_;
    // The cumulative distribution function value of the key
    double key_cdf_;
    // The cumulative distribution function for operation types
    map<double, OperationType> operation_cdf_;
    // The cumulative distribtuion function for the object sizes
    map<double, int> object_size_cdf_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
