/*
* Copyright (c) 2013, Facebook, Inc.
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*   * Neither the name Facebook nor the names of its contributors may be used to
*     endorse or promote products derived from this software without specific
*     prior written permission.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include <event2/event.h>
#include <gflags/gflags.h>
#include <pthread.h>

#include "Connection.h"
#include "Workload.h"

// Hostname of the server
DECLARE_string(hostname);
// Port number to connect
DECLARE_int32(port);
// Number of connections per thread
DECLARE_int32(number_of_connections);

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::deque;
using std::queue;
using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

// The class for worker thread
class Worker {
  public:
    /**
     * Contructor for Worker
     *
     * @param workload The workload object for this worker thread
     */
    Worker(shared_ptr<Workload> workload);
    /**
     * Main event_base loop for the worker thread
     */
    void mainLoop();
    /**
     * Call back fucntion for receive event
     *
     * @param fd The file descriptor for the connection
     */
    void receiveCallBack(int fd);
    /**
     * Call back function for send event
     *
     * @param fd The file descriptor for the connection
     */
    void sendCallBack(int fd);
    /**
     * Start the main event_base loop
     */
    void start();

  private:
    // A vector of connections wrapped in unique_ptr
    vector<unique_ptr<Connection> > connections_;
    // A fd to connection mapping
    unordered_map<int, int> connection_map_;
    // A vector of queues of outstanding requests indexed by connection
    vector<queue<shared_ptr<Request> > > request_queues_;
    // Pointer to the event_base struct
    struct event_base* event_base_;
    // Number of connections each worker thread handles
    int number_of_connections_;
    // pthread object for the worker thread
    unique_ptr<pthread_t> thread_;
    // Workload object which is shared among all worker threads
    shared_ptr<Workload> workload_;
};

/**
 * Handler function for mainLoop() to hook it up with libevent
 */
void* MainLoopHandler(void* args);
/**
 * Handler function for receiveCallBack() to hook it up with libevent
 */
void ReceiveCallBackHandler(int fd, short event_type, void* args);
/**
 * Handler function for sendCallBack() to hook it up with libevent
 */
void SendCallBackHandler(int fd, short event_type, void* args);

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
