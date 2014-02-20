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

#include "Connection.h"

#include <arpa/inet.h>
#include <cstdio>
#include <errno.h>
#include <glog/logging.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "Request.h"
#include "Util.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Constructor for Connection
 *
 * @param ip_address The IP address for the server in string
 * @param port Port number connecting to
 * @param disable_nagles Whether disable Nagle's algorithm
 * @return A connection set up to ip_address:port
 */
Connection::Connection(const string& ip_address,
                       int port,
                       bool disable_nagles) {
  // Allocate input and ouput buffers.
  read_buffer_.reset(new char[kBufferSize]);
  write_buffer_.reset(new char[kBufferSize]);
  value_buffer_.reset(new char[kBufferSize]);

  const string pattern = "test";
  for (int i = 0; i < kBufferSize / pattern.size(); i++) {
    memcpy(value_buffer_.get() + i * pattern.size(),
           pattern.c_str(),
           pattern.size());
  }

  // Create a new socket.
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ < 0) {
    LOG(FATAL) << "Could not create a socket";
  }

  struct sockaddr_in server_info;
  // Use IPv4.
  server_info.sin_family = AF_INET;

  // Convert IP address to network order
  if (inet_pton(AF_INET,
                ip_address.c_str(),
                &server_info.sin_addr.s_addr) < 0) {
    LOG(FATAL) << "IP address error";
  }

  // Connect to the supplied port.
  server_info.sin_port = htons(port);
  int error = connect(sock_,
                      reinterpret_cast<struct sockaddr*>(&server_info),
                      sizeof(server_info));
  if (error < 0) {
    LOG(FATAL) << "Connection error: " << strerror(errno);
  }

  // Disable nagles algorithm preventing batching.
  if (disable_nagles) {
    int flag = 1;
    int err = setsockopt(sock_,
                         IPPROTO_TCP,
                         TCP_NODELAY,
                         reinterpret_cast<char*>(&flag),
                         sizeof(int));
    if (err < 0) {
      LOG(FATAL) << "Could not set tcp_nodelay";
    }
  }
}

/**
 * Destructor for Connection
 */
Connection::~Connection() {
  close(sock_);
}

/**
 * Loop up the IP address given hostname
 *
 * @param hostname Hostname for the server in string
 * @return IP address under the hostname in string
 */
string Connection::nsLookUp(const string& hostname) {
  struct hostent* host_info = 0;
  for (int attempt = 0;
       (host_info == 0) && (attempt < kNumberOfAttempts);
       attempt++) {
    host_info = gethostbyname(hostname.c_str());
  }

  char* ip_address;
  if (host_info) {
    struct in_addr* address = (struct in_addr*)host_info->h_addr;
    ip_address = inet_ntoa(*address);
    LOG(INFO) << "Host: " << host_info->h_name;
    LOG(INFO) << "Address: " << ip_address;
  } else {
    LOG(FATAL) << "DNS error";
  }

  return string(ip_address);
}

/**
 * Receive response
 */
void Connection::receiveResponse() {
  int total_bytes_read = readLine(sock_, read_buffer_.get(), kBufferSize);

  // reponse for SET request
  if (read_buffer_.get()[0] == 'V') {
    int last_space_index = total_bytes_read - 2;
    while (read_buffer_.get()[last_space_index] != ' ') {
      last_space_index--;
    }
    int object_size = atoi(&read_buffer_.get()[last_space_index + 1]);
    // Read the result (+2 for \r\n)
    readBlock(sock_, read_buffer_.get(), object_size + 2);
    // Read END\r\n
    readLine(sock_, read_buffer_.get(), kBufferSize);
  }
}

/**
 * Send request
 */
void Connection::sendRequest(shared_ptr<Request> request) {
  // Write out key, flags exptime, and size.
  request->send(sock_, write_buffer_.get(), value_buffer_.get());
}

/**
 * Return the socket number of this connection
 *
 * @return Socket number of this connection
 */
int Connection::sock() {
  return sock_;
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
