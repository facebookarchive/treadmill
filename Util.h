/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once

#include <random>

#include <folly/dynamic.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

// Number of nanoseconds in one second.
constexpr int64_t k_ns_per_s = 1000000000;

/**
 * Get current time according to CLOCK_MONOTONIC.
 *
 * @return current time in nanoseconds.
 */
int64_t nowNs();

void writeDynamicToFile(std::string filename, folly::dynamic);

folly::dynamic readDynamicFromFile(std::string filename);

/**
 * Read a line from the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the read buffer
 * @return The total amount of bytes read
 */
int readLine(int fd, char* buffer, int buffer_size);

/**
 * Read from block given the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the read buffer
 */
void readBlock(int fd, char* buffer, int buffer_size);

/**
 * Write to block given the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the write buffer
 */
void writeBlock(int fd,
                const char* buffer,
                int buffer_size);

std::string nsLookUp(const std::string& hostname);

double time_s();

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
