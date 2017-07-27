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

using fb_status = facebook::fb303::cpp2::fb_status;

namespace facebook {
namespace windtunnel {
namespace treadmill {

TreadmillFB303::TreadmillFB303()
    : FacebookBase2("Treadmill"),
      status_(fb_status::STARTING),
      aliveSince_(time(nullptr)) {}

TreadmillFB303::~TreadmillFB303() {}

void TreadmillFB303::setStatus(fb_status status) {
  folly::SharedMutex::WriteHolder guard(mutex_);
  status_ = status;
}

fb_status TreadmillFB303::getStatus() {
  folly::SharedMutex::ReadHolder guard(mutex_);
  return status_;
}

void TreadmillFB303::getStatusDetails(std::string& _return) {
  _return = fb303::cpp2::_fb_status_VALUES_TO_NAMES.at(getStatus());
}

int64_t TreadmillFB303::aliveSince() {
  folly::SharedMutex::ReadHolder guard(mutex_);
  return aliveSince_;
}

void TreadmillFB303::getCounters(std::map<std::string, int64_t>& _return) {
  fb303::FacebookBase2::getCounters(_return);
}

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
