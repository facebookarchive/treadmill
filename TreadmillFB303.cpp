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

#include <folly/Conv.h>
#include <folly/Singleton.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include "common/services/cpp/TLSConfig.h"
#include "common/time/ClockGettimeNS.h"

DEFINE_bool(
    require_configuration_on_resume,
    false,
    "If true, 'resume' only when configuration is available");
DEFINE_bool(
    enable_watchdog_timer,
    false,
    "If true, a watchdog timer will be maintained during a run.");

using fb_status = facebook::fb303::cpp2::fb_status;
using ::treadmill::RateResponse;
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
      scheduler_(scheduler),
      configuration_(std::make_unique<std::map<std::string, std::string>>()),
      watchdogDurationSec_(0),
      lastHeartbeat_(0) {}

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
  _return = fb303::cpp2::_fb303_status_VALUES_TO_NAMES.at(getStatus());
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
  watchdogDurationSec_ = 0;
  return true;
}

bool TreadmillFB303::resume() {
  LOG(INFO) << "TreadmillHandler::resume";
  watchdogUpdate();
  folly::SharedMutex::ReadHolder guard(mutex_);
  if (FLAGS_require_configuration_on_resume && configuration_->empty()) {
    LOG(WARNING) << "refusing resume without configuration";
    return false;
  }
  return scheduler_.resume();
}

folly::Future<std::unique_ptr<ResumeResponse>> TreadmillFB303::future_resume2(
    std::unique_ptr<ResumeRequest> req) {
  // Get the phase name being super paranoid.
  auto phaseName = req != nullptr ? req->get_phaseName() : "UNKNOWN_PHASE";
  LOG(INFO) << "TreadmillHandler::resume2 with phase " << phaseName;
  watchdogUpdate();
  scheduler_.setPhase(phaseName);
  auto resp = std::make_unique<ResumeResponse>();
  auto running = scheduler_.resume();
  LOG(INFO) << "Scheduler is currently "
            << (running ? "Running" : "Not Running");
  resp->success_ref() = running;
  return folly::makeFuture(std::move(resp));
}

void TreadmillFB303::setRps(int32_t rps) {
  LOG(INFO) << "TreadmillHandler::setRps to " << rps;
  watchdogUpdate();
  scheduler_.setRps(rps);
}

void TreadmillFB303::setMaxOutstanding(int32_t max_outstanding) {
  LOG(INFO) << "TreadmillHandler::setMaxOutstanding to " << max_outstanding;
  watchdogUpdate();
  scheduler_.setMaxOutstandingRequests(max_outstanding);
}

folly::Future<std::unique_ptr<::treadmill::RateResponse>>
TreadmillFB303::future_getRate() {
  auto response = std::make_unique<RateResponse>();
  response->scheduler_running_ref() = scheduler_.isRunning();
  response->rps_ref() = scheduler_.getRps();
  response->max_outstanding_ref() = scheduler_.getMaxOutstandingRequests();
  return folly::makeFuture(std::move(response));
}

folly::Future<std::unique_ptr<std::string>>
TreadmillFB303::future_getConfiguration(std::unique_ptr<std::string> key) {
  LOG(INFO) << "TreadmillHandler::getConfiguration: " << *key;
  watchdogUpdate();

  folly::SharedMutex::ReadHolder guard(mutex_);
  if (configuration_->count(*key) > 0) {
    auto value = std::make_unique<std::string>(configuration_->at(*key));
    LOG(INFO) << "returning " << *key << " = " << *value;
    return folly::makeFuture(std::move(value));
  }
  auto value = std::make_unique<std::string>();
  return folly::makeFuture(std::move(value));
}

void TreadmillFB303::setConfiguration(
    std::unique_ptr<std::string> key,
    std::unique_ptr<std::string> value) {
  LOG(INFO) << "TreadmillHandler::setConfiguration: " << *key << " = "
            << *value;
  watchdogUpdate();

  folly::SharedMutex::WriteHolder guard(mutex_);
  configuration_->insert_or_assign(*key, *value);
  if (FLAGS_enable_watchdog_timer && *key == "watchdog_sec") {
    LOG(INFO) << "TreadmillHandler::watchdog timer value (secs) = " << *value;
    if (auto result = folly::tryTo<uint32_t>(*value)) {
      watchdogDurationSec_ = result.value();
    } else {
      watchdogDurationSec_ = 0; // disabled
    }
  }
}

uint32_t TreadmillFB303::getConfigurationValue(
    const std::string& key,
    uint32_t defaultValue) {
  folly::SharedMutex::ReadHolder guard(mutex_);
  if (configuration_->count(key) > 0) {
    auto value = std::make_unique<std::string>(configuration_->at(key));
    if (auto result = folly::tryTo<uint32_t>(*value)) {
      return result.value();
    }
    LOG(WARNING) << "failed to convert value [" << *value << "]";
    // fall through
  }
  return defaultValue;
}

std::unique_ptr<std::string> TreadmillFB303::getConfigurationValue(
    const std::string& key,
    const std::string& defaultValue) {
  folly::SharedMutex::ReadHolder guard(mutex_);
  if (configuration_->count(key) > 0) {
    return std::make_unique<std::string>(configuration_->at(key));
  } else {
    return std::make_unique<std::string>(defaultValue);
  }
}

void TreadmillFB303::clearConfiguration() {
  LOG(INFO) << "TreadmillHandler::clearConfiguration";
  watchdogUpdate();
  folly::SharedMutex::WriteHolder guard(mutex_);
  configuration_->clear();
}

bool TreadmillFB303::configurationEmpty() const {
  folly::SharedMutex::ReadHolder guard(mutex_);
  return configuration_->empty();
}

void TreadmillFB303::watchdogUpdate() {
  if (FLAGS_enable_watchdog_timer && watchdogDurationSec_ > 0) {
    lastHeartbeat_ = fb_time_seconds();
    LOG(INFO) << "watchdog update = " << lastHeartbeat_;
  }
}

bool TreadmillFB303::watchdogTimeoutCheck(bool raise) {
  if (FLAGS_enable_watchdog_timer && watchdogDurationSec_ > 0) {
    time_t now = fb_time_seconds();
    if (now - watchdogDurationSec_ > lastHeartbeat_) {
      LOG(WARNING) << "watchdog timeout: no contact since " << lastHeartbeat_;
      if (raise) {
        abort();
      }
      return true;
    }
  }
  return false;
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
