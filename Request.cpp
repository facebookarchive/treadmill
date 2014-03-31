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

#include "Request.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Constructor for Request
 *
 * @param key Key of the request
 * @param value_size Value size of the request
 */
Request::Request(const string& key, int value_size)
  : key_(key),
    value_size_(value_size) { }

/**
 * Get send time of a request
 *
 * @return The time when a request is sent
 */
struct timeval Request::send_time() {
  return send_time_;
}

/**
 * Set the send time of a request
 */
void Request::setSendTime() {
  struct timeval time_stamp;
  gettimeofday(&time_stamp, NULL);
  send_time_ = time_stamp;
}

// Define the request type map and initialize it
RequestTypeFactory::request_type_map*
RequestTypeFactory::request_type_map_ = new request_type_map();

// Define the request type vector in current workload and initialize it
vector<string> RequestTypeFactory::request_types_in_workload_;

/**
 * Static function to create a request by name
 *
 * @param request_type The type of the request in string
 * @param key Key of the request
 * @param value_size Value size of the request
 * @return A pointer to the created request
 */
Request* RequestTypeFactory::createRequestByName(const string& request_type,
                                                 const string& key,
                                                 int value_size) {
  request_type_map::iterator i = getRequestTypeMap()->find(request_type);
  if (i == getRequestTypeMap()->end()) {
    // Throw an exception if the request type has not been registered
    throw UnregisteredRequestTypeException();
  } else {
    return i->second(key, value_size);
  }
}

/**
 * Static function to initialize the map that contains all the request
 * types in a certain workload
 * 
 * @param workload_type The name of the workload in string
 */
void RequestTypeFactory::initializeRequestTypesByWorkload(
                              const string& workload_type) {
  for (request_type_map::iterator i = getRequestTypeMap()->begin();
       i != getRequestTypeMap()->end(); i++) {
    if (i->first.find(workload_type) == 0) {
      request_types_in_workload_.push_back(i->first);
    }
  }
}

/**
 * Static function to get a vector of all the request types in the
 * workload
 *
 * @return A vector of all the request types in string
 */
vector<string> RequestTypeFactory::request_types_in_workload() {
  return request_types_in_workload_;
}

/**
 * Static function to get a pointer to the global request type map
 *
 * @return A pointer to request_type_map_
 */
RequestTypeFactory::request_type_map* RequestTypeFactory::getRequestTypeMap() {
  if (!request_type_map_) {
    // This should not happen
    request_type_map_ = new RequestTypeFactory::request_type_map;
  }
  return request_type_map_;
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
