#include "Worker.h"

//#include "common/logging/logging.h"
#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

Worker::Worker() {

}

void Worker::start() {
  const string ipaddress = Connection::nslookup(FLAGS_hostname);
  Connection c(ipaddress, FLAGS_port);
  c.sendRequest();
  c.receiveResponse();
  bool running_;
  while (running_) {
    break;
  }
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
