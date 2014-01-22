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
#include "Util.h"

#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

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

/**
 * Constructor for SetRequest
 *
 * @param key The key of the request in string
 * @param value_size Size of the value for the SET request
 */
SetRequest::SetRequest(const string& key, int value_size) :
  key_(key),
  value_size_(value_size) { }

/**
 * Send method to send out the SET request
 *
 * @param fd File descriptor for the SET request
 * @param write_buffer The buffer to write operation, key, flags, etc.
 * @param value_buffer The buffer to write value for the SET operation
 */
void SetRequest::send(int fd, char* write_buffer, char* value_buffer) {
  // Write out key, flags exptime, and size.
  const string op = "set";
  const string key = key_;
  int flag = 0;
  int exptime = 0;
  int size = 10;
  int res = sprintf(write_buffer,
                    "%s %s %d %d %d\r\n",
                    op.c_str(),
                    key.c_str(),
                    flag,
                    exptime,
                    size);
  if (res < 0) {
    LOG(FATAL) << "Error with formatting key etc.";
  }
  writeBlock(fd, write_buffer, res);

  // Write out value
  res = sprintf(write_buffer,
                "%.*s\r\n",
                size,
                value_buffer);
  writeBlock(fd, write_buffer, res);

  // Set send time of the request
  setSendTime();
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
