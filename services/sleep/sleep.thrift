# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
include "common/fb303/if/fb303.thrift"

namespace cpp2 facebook.windtunnel.treadmill.services.sleep

service Sleep extends fb303.FacebookService {
  i64 goSleep(1: i64 time)
}
