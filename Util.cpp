/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "treadmill/Util.h"

#include <arpa/inet.h>
#include <fstream>
#include <netdb.h>
#include <sstream>

#include <sys/time.h>

#include <folly/json.h>
#include <glog/logging.h>

using std::string;

namespace facebook {
namespace windtunnel {
namespace treadmill {

// The number of attempts to get host information
const int kNumberOfAttempts = 3;

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
  std::string json = folly::toJson(object);
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
 * Loop up the IP address given hostname; return the first ip address
 * returned by getaddrinfo; if error occurs, return non-zero error code.
 *
 * @param hostname Hostname for the server in string, support ipv4 & ipv6
 * @return IP address under the hostname in string
 */
std::string nsLookUp(const string& hostname) {
  string ret;
  struct addrinfo hints;
  struct addrinfo *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */

  int error = -1;
  for (int attempt = 0;
       (error != 0) && (attempt < kNumberOfAttempts);
       attempt++
  ) {
    error = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
  }

  if (error == 0 ) {
    int len = res->ai_addrlen;

    if (res->ai_addr->sa_family == AF_INET) {
      // IPv4
      char ip_address[INET_ADDRSTRLEN];
      struct sockaddr_in* addr = (struct sockaddr_in*) res->ai_addr;
      inet_ntop(
        AF_INET,
        &(addr->sin_addr.s_addr),
        ip_address,
        INET_ADDRSTRLEN
      );
      ret = string(ip_address);
    } else {
      // IPv6
      char ip_address[INET6_ADDRSTRLEN];
      struct sockaddr_in6* addr = (struct sockaddr_in6*) res->ai_addr;
      inet_ntop(
        AF_INET6,
        &(addr->sin6_addr.s6_addr),
        ip_address,
        INET6_ADDRSTRLEN
      );
      ret = string(ip_address);
    }
    freeaddrinfo(res);
  } else {
    LOG(FATAL) << "DNS error: " << gai_strerror(error);
    exit(error);
  }

  return ret;
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
