
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "Connection.h"
#include "Worker.h"

using std::string;

DEFINE_string(hostname,
              "",
              "The host to load test.");
DEFINE_int32(port,
             0,
             "The port on the host to connect to.");

namespace facebook {
namespace windtunnel {
namespace treadmill {

int run(int argc, char *argv[]) {

  Worker w;
  w.start();

  LOG(INFO) << "Complete";

  return 0;
}

} // namespace treadmill
} // namespace windtunnel
} // namespace facebook

int main(int argc, char *argv[]) {
  //facebook::initFacebook(&argc, &argv);
  google::ParseCommandLineFlags(&argc, &argv, true);
  return facebook::windtunnel::treadmill::run(argc, argv);
}
