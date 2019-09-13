// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
include "common/fb303/if/fb303.thrift"

namespace cpp2 treadmill
namespace py treadmill

typedef string PhaseName

struct ResumeRequest {
  /**
   * Name of the phase that treadmill is transitioning to
   */
  1: required PhaseName phaseName;
}

struct ResumeResponse {
  /**
   * Value indicating whether the resume was successful
   */
  1: required bool success;
}

struct RateResponse {
  /**
   * The state of the scheduler
   */
  1: optional bool scheduler_running;

  /**
   * The request per second rate
   */
  2: optional i32 rps;

  /**
   * The maximum number of outstanding requests
   */
  3: optional i32 max_outstanding;
}

service TreadmillService extends fb303.FacebookService {
  bool pause();
  bool resume();
  /**
   * Temporary renaming of resume for backwards compatibility
   */
  ResumeResponse resume2(ResumeRequest req);
  void setRps(
    1: i32 rps
  );
  void setMaxOutstanding(
    1: i32 max_outstanding
  );
  RateResponse getRate();

  string getConfiguration(1: string key);
  void setConfiguration(1: string key, 2: string value);
  void clearConfiguration();
}
