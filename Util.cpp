
#include <glog/logging.h>
#include <string>
#include "Util.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::string;

void writeBlock(int fd,
                char* buffer,
                int buffer_size) {
  int total_bytes_written = 0;
  while (total_bytes_written != buffer_size) {
    int bytes_written = write(fd,
                              buffer + total_bytes_written,
                              buffer_size - total_bytes_written);
    // Write syscall failed.
    if (bytes_written < 0) {
      printf("Attempted write size %d\n", buffer_size - total_bytes_written);
      string sys_error = string(strerror(errno));
      LOG(FATAL) << "Write syscall failed: " + sys_error;
    }
    total_bytes_written += bytes_written;
  }
}

void readBlock(int fd, char* buffer, int buffer_size) {
  int total_bytes_read = 0;
  while (total_bytes_read != buffer_size) {
    int bytes_read = read(fd,
                          buffer + total_bytes_read,
                          buffer_size - total_bytes_read);
    // Read syscall failed.
    if (bytes_read < 0) {
      string sys_error = string(strerror(errno));
      LOG(FATAL) << "Read syscall failed: " + sys_error;
    }
    total_bytes_read += bytes_read;
  }

  // Make sure all bytes have been read from the fd.
  if (total_bytes_read < buffer_size) {
    LOG(FATAL) << "Read loop exited before all bytes were written."
                  "This should never happen";
  }
}


}
}
}
