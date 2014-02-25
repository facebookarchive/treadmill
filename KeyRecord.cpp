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

#include "KeyRecord.h"

#include "Util.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

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
KeyRecord::KeyRecord(const string& key, const double key_cdf,
                     const map<double, OperationType>& operation_cdf,
                     const map<double, int>& object_size_cdf)
  : key_(key),
    key_cdf_(key_cdf),
    operation_cdf_(operation_cdf),
    object_size_cdf_(object_size_cdf) { }

/**
 * Get the key of a record
 *
 * @return The key of a record
 */
string KeyRecord::key() {
  return this->key_;
}

/**
 * Get the cumulative distribution function value of a key
 *
 * @return The cumulative distribution function value
 */
double KeyRecord::key_cdf() {
  return this->key_cdf_;
}

/**
 * Get the cumulative distribution function for the operation types
 *
 * @return The cumulative distribution function for the operation
 *         types
 */
map<double, OperationType> KeyRecord::operation_cdf() {
  return this->operation_cdf_;
}
/**
 * Get the cumulative distribution function for the object sizes
 *
 * @return The cumulative distribution function for the object sizes
 */
map<double, int> KeyRecord::object_size_cdf() {
  return this->object_size_cdf_;
}

/**
 * Return a shared pointer to a randomly generated request
 *
 * @return A shared pointer to a randomly generated request
 */
shared_ptr<Request> KeyRecord::getRandomRequest() {
  if (this->getRandomOperation(RandomEngine::getDouble()) == GET_OPERATION) {
    return shared_ptr<GetRequest>(new GetRequest(this->key_));
  } else {
    int object_size = this->getRandomObjectSize(RandomEngine::getDouble());
    return shared_ptr<SetRequest>(new SetRequest(this->key_, object_size));
  }
}

/**
 * Return a shared pointer to a randomly generated warm-up request
 *
 * @return A shared pointer to a randomly generated warm-up request
 */
shared_ptr<Request> KeyRecord::getWarmUpRequest() {
  int object_size = this->getRandomObjectSize(RandomEngine::getDouble());
  return shared_ptr<SetRequest>(new SetRequest(this->key_, object_size));
}

/**
 * Get a random operation type based on the probability distribution
 * function for the operation types
 *
 * @param random_value A random value in [0.0, 1.0)
 * @return An operation type
 */
OperationType KeyRecord::getRandomOperation(const double random_value) {
  // Throw an exception if random_value is out of range [0.0, 1.0)
  if (random_value < 0.0 || random_value >= 1.0) {
    throw OutOfRangeRandomValueException();
  }

  // Generate operation type based on the probability distribution
  // function for the operation types
  auto bin_iter = operation_cdf_.lower_bound(random_value);
  if (bin_iter != operation_cdf_.end()) {
    return bin_iter->second;
  } else {
    // Throw an exception if the function does not return from the for loop
    // This usually occurs when the CDF value of the last element is not 1.0
    throw IllegalDistributionException();
  }
}

/**
 * Get a random object size based on the cumulative distribution
 * function for the object sizes
 *
 * @param random_value A random value in [0.0, 1.0)
 * @return An object size
 */
int KeyRecord::getRandomObjectSize(const double random_value) {
  // Throw an exception if random_value is out of range [0.0, 1.0)
  if (random_value < 0.0 || random_value >= 1.0) {
    throw OutOfRangeRandomValueException();
  }

  // Generate object size based on the cumulative distribution
  // function for the object sizes
  auto bin_iter = object_size_cdf_.lower_bound(random_value);
  if (bin_iter != object_size_cdf_.end()) {
    return bin_iter->second;
  } else {
    // Throw an exception if the function does not return from the for loop
    // This usually occurs when the CDF value of the last element is not 1.0
    throw IllegalDistributionException();
  }
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
