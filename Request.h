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
#include <string>
#include <vector>

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::string;
using std::unique_ptr;
using std::vector;

// Base class for all types of requests
class Request {
  public:
    /**
     * Constructor for Request
     */
    Request() { }
    /**
     * Virtual send method to send out send out the request
     *
     * @param fd File descriptor for the request
     * @param writer_buffer The buffer to write operation, key, flags, etc.
     * @param value_buffer The buffer to write value (e.g. SET operation)
     */
    virtual void send(int fd, char* write_buffer, char* value_buffer) = 0;
};

// Subclass for GET requests
class GetRequest : Request {
  public:
    /**
     * Constructor for GetRequest
     *
     * @param key The key of the request in string
     */
    explicit GetRequest(const string& key);
    /**
     * Send method to send out the GET request
     *
     * @param fd File descriptor for the GET request
     * @param write_buffer The buffer to write operation, key, flags, etc.
     * @param value_buffer The buffer to write value for the GET operation
     */
    void send(int fd, char* write_buffer, char* value_buffer) {}
};

// Subclass for SET requests
class SetRequest : Request {
  public:
    /**
     * Constructor for SetRequest
     *
     * @param key The key of the request in string
     * @param value_size Size of the value for the SET request
     */
    SetRequest(const string& key, int value_size);
    /**
     * Send method to send out the SET request
     *
     * @param fd File descriptor for the SET request
     * @param write_buffer The buffer to write operation, key, flags, etc.
     * @param value_buffer The buffer to write value for the SET operation
     */
    void send(int fd, char* write_buffer, char* value_buffer);
  private:
    // Key for the SET request
    string key_;
    // Size of the value for the SET request
    int value_size_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
