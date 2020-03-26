/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <gflags/gflags.h>

#include "treadmill/services/sleep/gen-cpp2/Sleep.h"
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>

using namespace folly;
using namespace apache::thrift;
using namespace facebook::windtunnel::treadmill::services::sleep;

DEFINE_int32(port,
             12345,
             "Port for sleep service (default: 12345).");

DEFINE_string(hostname,
              "127.0.0.1",
              "Hostname of the server (default: localhost).");

DEFINE_int32(sleep_time,
             1000,
             "The sleep time to send (default: 1000).");

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  EventBase event_base;

  std::shared_ptr<AsyncSocket> socket(
      AsyncSocket::newSocket(&event_base, FLAGS_hostname, FLAGS_port));

  SleepAsyncClient client(HeaderClientChannel::newChannel(socket));

  auto sleep_time = client.future_goSleep(FLAGS_sleep_time).getVia(&event_base);
  LOG(INFO) << "Slept for " << sleep_time << " microseconds.";
  return 0;
}
