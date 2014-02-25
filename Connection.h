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

#include <cstdlib>
#include <memory>
#include <string>

#include "Request.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::shared_ptr;
using std::string;
using std::unique_ptr;

// The maximal size of the read/value/write buffer
const int kBufferSize = 2 * 1024 * 1024;
// The number of attempts to get host information
const int kNumberOfAttempts = 3;

// A class to represent a connection to a server
class Connection {
  public:
    /**
     * Constructor for Connection
     *
     * @param ip_address A string containing the IP address of the server
     * @param port Port number of the server under test
     * @param disable_nagles Whether to disable Nagle's algorithm
     * @return A connection to ip_address:port
     */
    Connection(const string& ip_address, int port, bool disable_nagles=true);
    /**
     * Destructor for Connection, which closes the connection
     */
    virtual ~Connection();

    /**
     * Loop up the IP address given hostname
     *
     * @param hostname Hostname for the server in string
     * @return IP address under the hostname in string
     */
    static string nsLookUp(const string& hostname);
    /**
     * Receive a response
     *
     * @param request The request sent out for the response
     */
    void receiveResponse(shared_ptr<Request> request);
    /**
     * Send a request
     */
    void sendRequest(shared_ptr<Request> request);
    /**
     * Return the socket number of this connection
     *
     * @return Socket number of this connection
     */
    int sock();

  private:
    // Socket number for the connection
    int sock_;
    // Buffer used to read from fd
    unique_ptr<char> read_buffer_;
    // Buffer used to set value to fd
    unique_ptr<char> value_buffer_;
    // Buffer used to write to fd
    unique_ptr<char> write_buffer_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
