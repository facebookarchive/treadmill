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

#include "Scheduler.h"

#include <memory>

#include <folly/Singleton.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include "common/services/cpp/TLSConfig.h"

using fb_status = facebook::fb303::cpp2::fb_status;
using ::treadmill::ResumeRequest;
using ::treadmill::ResumeResponse;

using namespace facebook::services;

namespace facebook {
namespace windtunnel {
namespace treadmill {

TreadmillFB303::TreadmillFB303(Scheduler& scheduler)
    : FacebookBase2("Treadmill"),
      status_(fb_status::STARTING),
      aliveSince_(time(nullptr)),
      scheduler_(scheduler) {}

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

bool TreadmillFB303::pause() {
  LOG(INFO) << "TreadmillHandler::pause";
  scheduler_.pause();
  return true;
}

bool TreadmillFB303::resume() {
  LOG(INFO) << "TreadmillHandler::resume";
  return scheduler_.resume();
}

folly::Future<std::unique_ptr<ResumeResponse>> TreadmillFB303::future_resume2(
    std::unique_ptr<ResumeRequest> req) {
  // Get the phase name being super paranoid.
  auto phaseName = req != nullptr ? req->get_phaseName() : "UNKNOWN_PHASE";
  LOG(INFO) << "TreadmillHandler::resume2 with phase " << phaseName;
  scheduler_.setPhase(phaseName);
  auto resp = std::make_unique<ResumeResponse>();
  auto running = scheduler_.resume();
  LOG(INFO) << "Scheduler is currently "
            << (running ? "Running" : "Not Running");
  resp->set_success(running);
  return folly::makeFuture(std::move(resp));
}

void TreadmillFB303::setRps(int32_t rps) {
  LOG(INFO) << "TreadmillHandler::setRps to " << rps;
  scheduler_.setRps(rps);
}

void TreadmillFB303::setMaxOutstanding(int32_t max_outstanding) {
  LOG(INFO) << "TreadmillHandler::setMaxOutstanding to " << max_outstanding;
  scheduler_.setMaxOutstandingRequests(max_outstanding);
}

namespace {
folly::SharedMutex instance_mutex;
std::shared_ptr<TreadmillFB303> instance;
} // namespace

std::shared_ptr<TreadmillFB303> getGlobalTreadmillFB303() {
  folly::SharedMutex::ReadHolder guard(instance_mutex);
  if (!instance) {
    LOG(FATAL) << "No global Treadmill FB303 instance set";
  }

  return instance;
}

void TreadmillFB303::make_fb303(
    std::shared_ptr<std::thread>& server_thread,
    int server_port,
    Scheduler& scheduler) {
  {
    folly::SharedMutex::WriteHolder guard(instance_mutex);
    if (instance) {
      LOG(FATAL) << "Global Treadmill FB303 instance was already set";
    }
    instance = std::make_shared<TreadmillFB303>(scheduler);
  }

  auto server = std::make_shared<apache::thrift::ThriftServer>();
  LOG(INFO) << "FB303 running on port " << server_port;
  server->setPort(server_port);
  server->setInterface(getGlobalTreadmillFB303());
  TLSConfig::applyDefaultsToThriftServer(*server);
  server_thread.reset(
      new std::thread([server]() { server->serve(); }),
      [server](std::thread* t) {
        server->stop();
        t->join();
        delete t;
      });
}

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
