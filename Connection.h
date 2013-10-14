#pragma once

#include <memory>
#include <string>

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::unique_ptr;
using std::string;

const int kBufferSize = 2 * 1024 * 1024;

// A class to represent a connection to a server
class Connection {
 public:
  Connection(const string& ip_address, int port, bool disable_nagles=true);
  virtual ~Connection();
  static string nslookup(const string& hostname);
  void receiveResponse();
  void sendRequest();
 private:
  int sock_;
  unique_ptr<char> readBuffer_;
  unique_ptr<char> writeBuffer_;
  unique_ptr<char> valueBuffer_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook

