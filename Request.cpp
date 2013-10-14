
#include "Request.h"
#include "Util.h"

#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

SetRequest::SetRequest(const string& key, int valueSize) :
  key_(key),
  valueSize_(valueSize) {}


void SetRequest::send(int fd, char* writeBuffer, char* valueBuffer) {
  // Write out key, flags exptime, and size.
  const string op = "set";
  const string key = "foo";
  int flag = 0;
  int exptime = 0;
  int size = 10;
  int res = sprintf(writeBuffer,
                     "%s %s %d %d %d\r\n",
                      op.c_str(),
                      key.c_str(),
                      flag,
                      exptime,
                      size);
  if (res < 0) {
    LOG(FATAL) << "Error with formatting key etc.";
  }
  writeBlock(fd, writeBuffer, res);

  // Write out value
  res = sprintf(writeBuffer,
                    "%.*s\r\n",
                    size,
                    valueBuffer);
  writeBlock(fd, writeBuffer, res);
}

} // treadmill
} // windtunnel
} // facebook
