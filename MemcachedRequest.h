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

#include <string>

#include "Request.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::map;
using std::string;

// Subclass for Memcached GET requests
class MemcachedGetRequest : public Request {
  public:
    /**
     * Constructor for GetRequest
     *
     * @param key The key of the request in string
     */
    MemcachedGetRequest(const string& key, int value_size);
    /**
     * Send method to send out the GET request
     *
     * @param fd File descriptor for the GET request
     * @param write_buffer The buffer to write operation, key, flags, etc.
     * @param value_buffer The buffer to write value for the GET operation
     */
    void send(int fd, char* write_buffer, char* value_buffer);
    /**
     * Receive method to receive response for a request
     *
     * @param fd File descriptor for the request
     * @param read_buffer The buffer to read the response
     * @param kBufferSize The size of the read buffer
     */
    void receive(int fd, char* read_buffer, const int kBufferSize);
    /**
     * Virtual get method that returns the request type
     *
     * @return The type of the request
     */
    string getRequestType();

  private:
    // Register MemcachedGetRequest
    REGISTER_DECLARE_REQUEST_TYPE(MemcachedGetRequest);
};

// Subclass for SET requests
class MemcachedSetRequest : public Request {
  public:
    /**
     * Constructor for SetRequest
     *
     * @param key The key of the request in string
     * @param value_size Size of the value for the SET request
     */
    MemcachedSetRequest(const string& key, int value_size);
    /**
     * Send method to send out the SET request
     *
     * @param fd File descriptor for the SET request
     * @param write_buffer The buffer to write operation, key, flags, etc.
     * @param value_buffer The buffer to write value for the SET operation
     */
    void send(int fd, char* write_buffer, char* value_buffer);
    /**
     * Receive method to receive response for a request
     *
     * @param fd File descriptor for the request
     * @param read_buffer The buffer to read the response
     * @param kBufferSize The size of the read buffer
     */
    void receive(int fd, char* read_buffer, const int kBufferSize);
    /**
     * Virtual get method that returns the request type
     *
     * @return The type of the request
     */
    string getRequestType();

  private:
    // Register MemcachedSetRequest
    REGISTER_DECLARE_REQUEST_TYPE(MemcachedSetRequest);
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
