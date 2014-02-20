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

#include <glog/logging.h>
#include <string>

#include "Util.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

// Seed the random engine
mt19937_64 RandomEngine::random_engine_(time(NULL));
// Generate a uniform distribution
uniform_real_distribution<double>
  RandomEngine::uniform_distribution_(0.0, 1.0);

/**
 * Return a random number ranging in [0.0, 1.0] in double
 *
 * @return A random number ranging in [0.0, 1.0] in double
 */
double RandomEngine::getDouble() {
  return uniform_distribution_(random_engine_);
}

/**
 * Read a line from the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the read buffer
 * @return The total amount of bytes read
 */
int readLine(int fd, char* buffer, int buffer_size) {
  int total_bytes_read = 0;
  do {
    read(fd, buffer + total_bytes_read, 1);
    total_bytes_read++;
  } while (total_bytes_read < buffer_size &&
           (buffer[total_bytes_read - 2] != '\r' ||
            buffer[total_bytes_read - 1] != '\n'));
  return total_bytes_read;
}

/**
 * Read from block given the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the read buffer
 */
void readBlock(int fd, char* buffer, int buffer_size) {
  int total_bytes_read = 0;
  while (total_bytes_read != buffer_size) {
    int bytes_read = read(fd,
                          buffer + total_bytes_read,
                          buffer_size - total_bytes_read);
    // Read syscall failed.
    if (bytes_read < 0) {
      string sys_error = string(strerror(errno));
      LOG(FATAL) << "Read syscall failed: " + sys_error;
    }
    total_bytes_read += bytes_read;
  }

  // Make sure all bytes have been read from the fd.
  if (total_bytes_read < buffer_size) {
    LOG(FATAL) << "Read loop exited before all bytes were written."
                  "This should never happen";
  }
}

/**
 * Write to block given the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the write buffer
 */
void writeBlock(int fd,
                const char* buffer,
                int buffer_size) {
  int total_bytes_written = 0;
  while (total_bytes_written != buffer_size) {
    int bytes_written = write(fd,
                              buffer + total_bytes_written,
                              buffer_size - total_bytes_written);
    // Write syscall failed.
    if (bytes_written < 0) {
      LOG(INFO) << "Attempted write size "
                << (buffer_size - total_bytes_written);
      string sys_error = string(strerror(errno));
      LOG(FATAL) << "Write syscall failed: " + sys_error;
    }
    total_bytes_written += bytes_written;
  }
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
