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
#include "Util.h"

#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Contructor for Worker
 *
 * @param workload The workload object for this worker thread
 * @param statistic The shared statistic pointer for the worker thread
 * @param interarrival_distribution The interarrival distribution
 * @param worker_id The ID of the worker in [0, number_of_workers)
 * @param number_of_workers Total number of workers
 */
Worker::Worker(shared_ptr<Workload> workload, shared_ptr<Statistic> statistic,
               shared_ptr<Distribution> interarrival_distribution,
               const int worker_id, const int number_of_workers)
  : event_base_(event_base_new()),
    number_of_connections_(FLAGS_number_of_connections),
    thread_(unique_ptr<pthread_t>(new pthread_t())),
    workload_(workload),
    statistic_(statistic),
    interarrival_distribution_(interarrival_distribution),
    request_index_(0) {
  const string ip_address = Connection::nsLookUp(FLAGS_hostname);
  for(int i = 0; i < number_of_connections_; i++) {
    connections_.push_back(
        unique_ptr<Connection>(new Connection(ip_address, FLAGS_port)));
    request_queues_.push_back(queue<shared_ptr<Request> >());
    connection_map_[connections_[i]->sock()] = i;
    struct timeval time_value;
    gettimeofday(&time_value, NULL);
    last_send_times_.push_back(time_value);
    interarrival_times_.push_back(-1.0);
  }
  warm_up_requests_ = workload_->generateWarmUpRequests(worker_id,
                                                        number_of_workers);
  workload_requests_ = workload_->generateRandomRequests(FLAGS_number_of_keys);
}

/**
 * Warm-up event_base for the worker thread
 */
void Worker::warmUpMainLoop() {
  for (int i = 0; i < number_of_connections_; i++) {
    LOG(INFO) << "Creating socket on fd: " << connections_[i]->sock();
    struct event* send_event = event_new(event_base_,
                                         connections_[i]->sock(),
                                         EV_WRITE | EV_PERSIST,
                                         WarmUpSendCallBackHandler,
                                         this);
    event_priority_set(send_event, 2);
    event_add(send_event, NULL);

    struct event* receive_event = event_new(event_base_,
                                            connections_[i]->sock(),
                                            EV_READ | EV_PERSIST,
                                            WarmUpReceiveCallBackHandler,
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
 * Call back function for receive event during warm-up phase
 *
 * @param fd The file descriptor for the connection
 */
void Worker::warmUpReceiveCallBack(int fd) {
  int connection_id = connection_map_[fd];
  shared_ptr<Request> request = request_queues_[connection_id].front();
  request_queues_[connection_id].pop();
  connections_[connection_id]->receiveResponse(request);

  // Check if all the warm-up requests have been sent out
  if (warm_up_requests_.size() == 0) {
    for (unordered_map<int, int>::iterator i = connection_map_.begin();
         i != connection_map_.end(); i++) {
      // Return if there are still requests in the queue
      if (request_queues_[i->second].size() != 0) {
        return ;
      }
    }
    // Break out the event_base loop
    event_base_loopbreak(event_base_);
  }
}

/**
 * Call back function for send event during warm-up phase
 *
 * @param fd The file descriptor for the connection
 */
void Worker::warmUpSendCallBack(int fd) {
  // Return if there are not any more warm-up requests
  if (warm_up_requests_.size() == 0) {
    return ;
  }

  int connection_id = connection_map_[fd];
  shared_ptr<Request> request = warm_up_requests_.top();
  warm_up_requests_.pop();
  connections_[connection_id]->sendRequest(request);
  request_queues_[connection_id].push(request);
}

/**
 * Warm Up the cache with event_base loop
 */
void Worker::warmUp() {
  this->warmUpMainLoop();
}

/**
 * Main event_base loop for the worker thread
 */
void Worker::mainLoop() {
  // Free the current event_base
  event_base_free(event_base_);
  // Create the new event_base for actual load testing
  event_base_ = event_base_new();

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
  connections_[connection_id]->receiveResponse(request);

  // Calculate the request latency
  struct timeval time_stamp, time_diff;
  gettimeofday(&time_stamp, NULL);
  struct timeval send_time = request->send_time();
  timersub(&time_stamp, &send_time, &time_diff);
  double request_latency = time_diff.tv_sec * 1e6 + time_diff.tv_usec;

  statistic_->addSample(request_latency, request->getRequestType());
}

/**
 * Call back function for send event
 */
void Worker::sendCallBack(int fd) {
  int connection_id = connection_map_[fd];

  // Get the time difference from last request sent
  struct timeval time_value, time_diff; 
  gettimeofday(&time_value, NULL);
  timersub(&time_value, &last_send_times_[connection_id], &time_diff);
  double diff = time_diff.tv_sec * 1e6  + time_diff.tv_usec;

  // Check if we waited enough interarrival time
  if (interarrival_times_[connection_id] < 0.0) {
    int quantile_index =
      (int) (RandomEngine::getDouble() * (kNumberOfValues - 1));
    double interarrival_time =
      interarrival_distribution_->getQuantileByIndex(quantile_index);
    interarrival_times_[connection_id] = interarrival_time;
  }
  if (diff < interarrival_times_[connection_id]) {
    return ;
  }

  shared_ptr<Request> request = workload_requests_[request_index_++];
  if (request_index_ == FLAGS_number_of_keys) {
    request_index_ = 0;
  }
  connections_[connection_id]->sendRequest(request);
  request_queues_[connection_id].push(request);
  interarrival_times_[connection_id] = -1.0;
  last_send_times_[connection_id] = request->send_time();
}

/**
 * Start the main event_base loop
 */
void Worker::start() {
  int rc = pthread_create(thread_.get(), NULL, MainLoopHandler, this);
  if (rc) {
    LOG(FATAL) << "Thread failed to start";
  }
}

/**
 * Stop the main event_base loop
 */
void Worker::stop() {
  // Break out the event_base loop
  event_base_loopbreak(event_base_);
  // Free the current event_base
  event_base_free(event_base_);
}

/**
 * Handler function for warmUpReceiveCallBack() to hook it up with libevent
 */
void WarmUpReceiveCallBackHandler(int fd, short event_type, void* args) {
  Worker* worker = static_cast<Worker*>(args);
  worker->warmUpReceiveCallBack(fd);
}

/**
 * Handler function for warmUpSendCallBack() to hook it up with libevent
 */
void WarmUpSendCallBackHandler(int fd, short event_type, void* args) {
  Worker* worker = static_cast<Worker*>(args);
  worker->warmUpSendCallBack(fd);
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
void ReceiveCallBackHandler(int fd, short event_type, void* args) {
  Worker* worker = static_cast<Worker*>(args);
  worker->receiveCallBack(fd);
}

/**
 * Handler function for sendCallBack() to hook it up with libevent
 */
void SendCallBackHandler(int fd, short event_type, void* args) {
  Worker* worker = static_cast<Worker*>(args);
  worker->sendCallBack(fd);
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
