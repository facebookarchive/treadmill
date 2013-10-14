
#include "Connection.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdio>
#include <string.h>

#include <glog/logging.h>
#include "Request.h"
#include "Util.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

void Connection::sendRequest() {
  // Write out key, flags exptime, and size.
  SetRequest sr("foo", 10);
  sr.send(sock_, writeBuffer_.get(), valueBuffer_.get());
}

void Connection::receiveResponse() {
  int nBytes = read(sock_, readBuffer_.get(), kBufferSize);
  readBlock(sock_, readBuffer_.get(), kBufferSize);
  printf("result: %s", readBuffer_.get());

}

string Connection::nslookup(const string& hostname) {
  struct hostent* host_info = 0;
  for (int attempt = 0; (host_info == 0) && (attempt < 3); attempt++) {
    host_info = gethostbyname(hostname.c_str());
  }

  char* ip_address;
  if (host_info) {
    struct in_addr* address = (struct in_addr*)host_info->h_addr;
    ip_address = inet_ntoa(*address);
    printf("Host: %s\n", host_info->h_name);
    printf("Address: %s\n", ip_address);
  } else {
    LOG(FATAL) << "DNS error";
  }
  return string(ip_address);
}

Connection::Connection(const string& ip_address,
                       int port,
                       bool disable_nagles) {
  // Allocate input and ouput buffers.
  readBuffer_.reset(new char[kBufferSize]);
  writeBuffer_.reset(new char[kBufferSize]);
  valueBuffer_.reset(new char[kBufferSize]);
  const string pattern = "test";
  for (int i = 0; i < kBufferSize / pattern.size(); i++) {
    memcpy(valueBuffer_.get() + i * pattern.size(),
           pattern.c_str(),
           pattern.size());
  }

  // Create a new socket.
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ < 0) {
    LOG(FATAL) << "Could not create a socket";
  }

  struct sockaddr_in server_info;
  // Use IPv4.
  server_info.sin_family = AF_INET;

  // Convert IP address to network order
  if (inet_pton(AF_INET,
                ip_address.c_str(),
                &server_info.sin_addr.s_addr) < 0) {
    LOG(FATAL) << "IP address error";
  }

  // Connect to the supplied port.
  server_info.sin_port = htons(port);
  int error = connect(sock_,
                      reinterpret_cast<struct sockaddr*>(&server_info),
                      sizeof(server_info));
  if (error < 0) {
    printf("Connection error: %s\n", strerror(errno));
    LOG(FATAL) << "Connection error";
  }

  // Disable nagles algorithm preventing batching.
  if (disable_nagles) {
    int flag = 1;
    int err = setsockopt(sock_,
                         IPPROTO_TCP,
                         TCP_NODELAY,
                         reinterpret_cast<char*>(&flag),
                         sizeof(int));
    if (err < 0) {
      LOG(FATAL) << "Could not set tcp_nodelay";
    }
  }
}

Connection::~Connection() {
  close(sock_);
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
