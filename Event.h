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

#include <folly/dynamic.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

enum class EventType {
  STOP,
  RESET,
  SEND_REQUEST,
};

class Event {
 public:
  explicit Event(EventType event_type, folly::dynamic extra_data = nullptr)
      : eventType_(event_type), extraData_(extra_data) {}
  EventType getEventType() const {
    return eventType_;
  }
  const folly::dynamic getExtraData() const {
    return extraData_;
  }

 private:
  EventType eventType_;
  folly::dynamic extraData_;
};

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook
