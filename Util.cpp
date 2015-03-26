/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "Util.h"

#include <arpa/inet.h>
#include <fstream>
#include <netdb.h>
#include <glog/logging.h>
#include <sys/time.h>

#include <folly/json.h>

using std::mt19937_64;
using std::string;
using std::uniform_real_distribution;

namespace facebook {
namespace windtunnel {
namespace treadmill {

// The number of attempts to get host information
const int kNumberOfAttempts = 3;

// Seed the random engine
mt19937_64 RandomEngine::random_engine_(time(nullptr));
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

int64_t nowNs() {
  struct timespec ts;
  int r = clock_gettime(CLOCK_MONOTONIC, &ts);
  PCHECK(r == 0);
  return (ts.tv_nsec + ts.tv_sec * k_ns_per_s);
}

bool writeStringToFile(std::string txt, std::string filename) {
  std::ofstream os(filename);
  try {
    os.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    os << txt;
    os.close();
  } catch (std::exception const& e) {
    LOG(ERROR) << "Failed to write file " << e.what();
    return false;
  }

  return true;
}

bool readFileToString(std::string filename, std::string& txt) {
  std::ifstream is(filename.c_str());
  try {
    is.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::stringstream buffer;
    buffer << is.rdbuf();
    is.close();
    txt = buffer.str();
  } catch (std::exception const& e) {
    LOG(ERROR) << "Failed to read file " << e.what();
    return false;
  }

  return true;
}

void writeDynamicToFile(std::string filename, folly::dynamic object) {
  std::string json = folly::toJson(object).toStdString();
  if (!writeStringToFile(json, filename)) {
    LOG(FATAL) << "Open to read failed: " << filename;
  }
}

folly::dynamic readDynamicFromFile(std::string filename) {
  std::string s;
  if (!readFileToString(filename, s)) {
    LOG(FATAL) << "Open to read failed: " << filename;
  }
  return folly::parseJson(s);
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

double time_s() {
  struct timeval time_stamp = {0, 0};
  gettimeofday(&time_stamp, nullptr);
  return time_stamp.tv_sec + time_stamp.tv_usec * 1e-6;
}

/**
 * Loop up the IP address given hostname
 *
 * @param hostname Hostname for the server in string
 * @return IP address under the hostname in string
 */
std::string nsLookUp(const string& hostname) {
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
  } else {
    LOG(FATAL) << "DNS error";
  }

  return string(ip_address);
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
