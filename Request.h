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

#include <sys/time.h>

// Macro to declare a request type (should be a private member of the class)
#define REGISTER_DECLARE_REQUEST_TYPE(NAME) \
  static RequestTypeRegister<NAME> request_type_register

// Macro to define a request type (should be put in the cpp file)
#define REGISTER_DEFINE_REQUEST_TYPE(NAME) \
  RequestTypeRegister<NAME> NAME::request_type_register(#NAME)

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::exception;
using std::make_pair;
using std::map;
using std::iterator;
using std::shared_ptr;
using std::string;
using std::vector;

// Class for unregistered request type exception
class UnregisteredRequestTypeException : public exception {
  const char* what () const throw () {
    return "Unregistered request type being called";
  }
};

// Base class for all types of requests
class Request {
  public:
    /**
     * Constructor for Request
     *
     * @param key Key of the request
     * @param value_size Value size of the request
     */
    Request(const string& key, int value_size);
    /**
     * Virtual send method to send out send out the request
     *
     * @param fd File descriptor for the request
     * @param writer_buffer The buffer to write operation, key, flags, etc.
     * @param value_buffer The buffer to write value (e.g. SET operation)
     */
    virtual void send(int fd, char* write_buffer, char* value_buffer) = 0;
    /**
     * Virtual receive method to receive response for a request
     *
     * @param fd File descriptor for the request
     * @param read_buffer The buffer to read the response
     * @param kBufferSize The size of the read buffer
     */
    virtual void receive(int fd, char* read_buffer, const int kBufferSize) = 0;
    /**
     * Virtual get method that returns the request type
     *
     * @return The type of the request
     */
    virtual string getRequestType() = 0;
    /**
     * Get send time of a request
     *
     * @return The time when a request is sent
     */
    struct timeval send_time();

  protected:
    /**
     * Set the send time of a request
     */
    void setSendTime();

    // Key for the request
    string key_;
    // Size of the value for the request
    int value_size_;

  private:
    // Time when the request is sent
    struct timeval send_time_;
};

/**
 * Template function to create a request
 *
 * @param key Key of the request
 * @param value_size Value size of the request
 */
template<typename T>
Request* createRequest(const string& key, int value_size) {
  return new T(key, value_size);
}

// Factory struct for different types of request
struct RequestTypeFactory {
  public:
    // Define the type of the request type map
    typedef map<string, Request*(*)(const string&, int)> request_type_map;
    /**
     * Static function to create a request by name
     *
     * @param request_type The type of the request in string
     * @param key Key of the request
     * @param value_size Value size of the request
     * @return A pointer to the created request
     */
    static Request* createRequestByName(const string& request_type,
                                        const string& key,
                                        int value_size);
    /**
     * Static function to initialize the map that contains all the request
     * types in a certain workload
     * 
     * @param workload_type The name of the workload in string
     */
    static void initializeRequestTypesByWorkload(const string& workload_type);
    /**
     * Static function to get a vector of all the request types in the
     * workload
     *
     * @return A vector of all the request types in string
     */
    static vector<string> request_types_in_workload();

  protected:
    /**
     * Static function to get a pointer to the global request type map
     *
     * @return A pointer to request_type_map_
     */
    static request_type_map* getRequestTypeMap();

  private:
    // Static request type map
    static request_type_map* request_type_map_;
    // Static vector of all the request types in certain workload
    static vector<string> request_types_in_workload_;
};

// Template struct for request type register
template<typename T> struct RequestTypeRegister : RequestTypeFactory {
  public:
    /**
     * Function to register the request type
     *
     * @param request_type The name of the request type in string
     */
    RequestTypeRegister(const string& request_type) {
      // Create an entry in the request type map
      getRequestTypeMap()->insert(make_pair(request_type,
                                            &createRequest<T>));
    }
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
