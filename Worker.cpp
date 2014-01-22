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

#include "Worker.h"

#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Contructor for Worker
 *
 * @param workload The workload object for this worker thread
 */
Worker::Worker(shared_ptr<Workload> workload)
  : event_base_(event_base_new()),
    number_of_connections_(FLAGS_number_of_connections),
    thread_(unique_ptr<pthread_t>(new pthread_t())),
    workload_(workload) {
  const string ip_address = Connection::nsLookUp(FLAGS_hostname);
  for(int i = 0; i < number_of_connections_; i++) {
    connections_.push_back(
        unique_ptr<Connection>(new Connection(ip_address, FLAGS_port)));
    request_queues_.push_back(queue<shared_ptr<Request> >());
    connection_map_[connections_[i]->sock()] = i;
  }
}

/**
 * Main event_base loop for the worker thread
 */
void Worker::mainLoop() {
  for (int i = 0; i < number_of_connections_; i++) {
    LOG(INFO) << "Creating socket on fd: " << connections_[i]->sock();
    struct event* send_event = event_new(event_base_,
                                         connections_[i]->sock(),
                                         EV_WRITE | EV_PERSIST,
                                         SendCallBackHandler,
                                         this);
    event_priority_set(send_event, 2);
    event_add(send_event, NULL);

    struct event* receive_event = event_new(event_base_,
                                            connections_[i]->sock(),
                                            EV_READ | EV_PERSIST,
                                            ReceiveCallBackHandler,
                                            this);
    event_priority_set(receive_event, 1);
    event_add(receive_event, NULL);
  }

  int error = event_base_loop(event_base_, 0);
  if (error == -1) {
    LOG(FATAL) << "Error starting libevent";
  } else if (error == 1) {
    LOG(FATAL) << "No events registered with libevent";
  }
}

/**
 * Call back fucntion for receive event
 */
void Worker::receiveCallBack(int fd) {
  int connection_id = connection_map_[fd];
  shared_ptr<Request> request = request_queues_[connection_id].front();
  request_queues_[connection_id].pop();

  // Calculate the request latency
  struct timeval time_stamp, time_diff;
  gettimeofday(&time_stamp, NULL);
  struct timeval send_time = request->send_time();
  timersub(&time_stamp, &send_time, &time_diff);
  double request_latency = time_diff.tv_usec * 1e-6  + time_diff.tv_sec;
}

/**
 * Call back function for send event
 */
void Worker::sendCallBack(int fd) {
  int connection_id = connection_map_[fd];
  shared_ptr<SetRequest> set_request (new SetRequest("foo", 10));
  connections_[connection_id]->sendRequest(set_request);
  request_queues_[connection_id].push(set_request);
}

/**
 * Start the main event_base loop
 */
void Worker::start() {
  int rc = pthread_create(thread_.get(), NULL, MainLoopHandler, this);
  if (rc) {
    LOG(FATAL) << "Thread failed to start";
  }
  // Loop forever for now
  while (1) { };
}

/**
 * Handler function for mainLoop() to hook it up with libevent
 */
void* MainLoopHandler(void* args) {
  Worker* worker = static_cast<Worker*>(args);
  worker->mainLoop();
  return NULL;
}

/**
 * Handler function for receiveCallBack() to hook it up with libevent
 */
void SendCallBackHandler(int fd, short event_type, void* args) {
  Worker* worker = static_cast<Worker*>(args);
  worker->sendCallBack(fd);
}

/**
 * Handler function for sendCallBack() to hook it up with libevent
 */
void ReceiveCallBackHandler(int fd, short event_type, void* args) {
  Worker* worker = static_cast<Worker*>(args);
  worker->receiveCallBack(fd);
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
