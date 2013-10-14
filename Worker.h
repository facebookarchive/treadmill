#pragma once

#include <memory>
#include <vector>

#include <gflags/gflags.h>
#include "Connection.h"

DECLARE_string(hostname);
DECLARE_int32(port);

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::unique_ptr;
using std::vector;

class Worker {
 public:
  Worker();
  void start();
 private:
  vector<unique_ptr<Connection>> connections;
  bool running_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook

