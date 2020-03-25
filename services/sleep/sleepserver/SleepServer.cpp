/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <memory>
#include <gflags/gflags.h>

#include <thrift/lib/cpp2/server/ThriftServer.h>

#include "treadmill/services/sleep/sleepserver/SleepHandler.h"

using namespace apache::thrift;

using namespace facebook::windtunnel::treadmill::services::sleep;

DEFINE_int32(port,
             12345,
             "Port for sleep service (default: 12345).");

DEFINE_int32(num_workers,
             4,
             "Number of workers (default: 4).");

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  auto handler = std::make_shared<SleepHandler>();
  auto server = std::make_unique<ThriftServer>();
  server->setPort(FLAGS_port);
  server->setNumIOWorkerThreads(FLAGS_num_workers);
  server->setInterface(std::move(handler));

  server->serve();

  return 0;
}
