/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "TreadmillFB303.h"

#include <memory>

#include <folly/Singleton.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

namespace {
folly::Singleton<TreadmillFB303> instance;
}

std::shared_ptr<TreadmillFB303> getGlobalTreadmillFB303() {
  return instance.try_get();
}

void TreadmillFB303::make_fb303(
    std::shared_ptr<std::thread>& server_thread,
    int server_port) {
  using facebook::windtunnel::treadmill::getGlobalTreadmillFB303;
  auto server = std::make_shared<apache::thrift::ThriftServer>();
  LOG(INFO) << "FB303 running on port " << server_port;
  server->setPort(server_port);
  server->setInterface(getGlobalTreadmillFB303());
  server_thread.reset(
    new std::thread(
      [server]() {
        server->serve();
      }),
    [server](std::thread* t) {
      server->stop();
      t->join();
      delete t;
    }
  );
}

}
}
}
