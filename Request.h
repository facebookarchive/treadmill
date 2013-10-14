#pragma once

#include <memory>
#include <string>
#include <vector>

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::unique_ptr;
using std::string;
using std::vector;

class Request {
 public:
  Request() {}
  virtual void send(int fd, char* writeBuffer, char* valueBuffer) = 0;
};

class SetRequest : Request {
 public:
  SetRequest(const string& key, int valueSize);
  void send(int fd, char* writeBuffer, char* valueBuffer);
 private:
  string key_;
  int valueSize_;
};

class GetRequest : Request {
 public:
  explicit GetRequest(const string& key);
  void send(int fd, char* writeBuffer, char* valueBuffer) {}
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook

