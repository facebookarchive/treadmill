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

#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include <folly/dynamic.h>
#include <folly/json.h>

#include "KeyRecord.h"

namespace facebook {
namespace windtunnel {
namespace treadmill {

using folly::dynamic;
using folly::parseJson;

using std::ifstream;
using std::istreambuf_iterator;
using std::make_pair;
using std::map;
using std::shared_ptr;
using std::stack;
using std::string;
using std::stringstream;
using std::to_string;
using std::unique_ptr;
using std::vector;

// Class for storing the workload characteristics (e.g. operation distribution)
class Workload {
  public:
    /**
     * Constructor for Workload
     */
    Workload();

    /**
     * Generator method taking a set of parameters including number of keys,
     * operation type CDF and MIN/MAX object size. This method will create
     * a workload with uniformly distributed key popularity, and each key will
     * have the same operation CDF and a uniformly distributed object size
     *
     * @param number_of_keys The number of keys for the workload
     * @param operation_cdf The cumulative distribution function for the
     *                      operation
     * @param min_object_size The minimal size of an object
     * @param max_object_size The maximal size of an object
     * @return A shared pointer to 
     */
    static shared_ptr<Workload> generateWorkloadByParameter(
                                const long number_of_keys,
                                const map<double, string> operation_cdf,
                                const int min_object_size,
                                const int max_object_size);
    /**
     * Generator method taking a JSON configuration file which contains
     * workload characteristics, and scale up/down the number of keys to
     * the amount needed
     *
     * @param number_of_keys The number of keys for the workload
     * @param config_file_path The path to the JSON configuration file
     * @return A shared pointer to a Workload object
     */
    static shared_ptr<Workload> generateWorkloadByConfigFile(
                                  const long number_of_keys,
                                  const string& config_file_path);
    /**
     * Return the number of keys in the workload
     *
     * @return The number of keys in the workload
     */
    long number_of_keys();
    /**
     * Return a map to the average operation PDF
     *
     * @return A map to the average operation PDF
     */
    map<string, double> average_operation_pdf();
    /**
     * Return the average object size in the workload
     *
     * @return The average object size
     */
    double average_object_size();
    /**
     * Return a vector of shared pointers to randomly generated requests
     *
     * @param number_of_requests The number of requests to generate
     * @return A vector of shared pointers to randomly generated requests
     */
    vector<shared_ptr<Request> > generateRandomRequests(
                                  const long number_of_requests);
    /**
     * Return a stack of shared pointers to warm-up requests of the workload
     * for a particular worker, and the requests are sorted from lowest PDF
     * to highest
     *
     * @param worker_id The ID of the current worker [0, number_of_workers)
     * @param number_of_workers Total number of workers
     * @return A stack of shared pointers to warm-up requests
     */
    stack<shared_ptr<Request> > generateWarmUpRequests(
                                  const int worker_id,
                                  const int number_of_workers);

  private:
    /**
     * Scale down the number of keys when less keys are needed,
     * and keep it same if the it is correct
     *
     * @param original_number_of_keys The original number of keys in the config
     *                                file
     * @param number_of_keys The number of keys needed
     * @param workload_config The parsed JSON workload configuration
     * @return A shared pointer to a Workload object
     */
    static shared_ptr<Workload> scaleDownNumberOfKeys(
                                  const long original_number_of_keys,
                                  const long number_of_keys,
                                  const dynamic& workload_config);
    /**
     * Scale up the number of keys when more keys are needed
     *
     * @param original_number_of_keys The original number of keys in the config
     *                                file
     * @param number_of_keys The number of keys needed
     * @param workload_config The parsed JSON workload configuration
     * @return A shared pointer to a Workload object
     */
    static shared_ptr<Workload> scaleUpNumberOfKeys(
                                  const long original_number_of_keys,
                                  const long number_of_keys,
                                  const dynamic& workload_config);
    /**
     * Merge multiple keys into one to scale down the number of keys
     *
     * @param base_key_index The index of the first key to merge
     * @param number_of_keys_in_group The number of keys to merge
     * @param base_key_cdf The key CDF value of the previous key
     * @param workload_config The parsed JSON workload configuration
     * @return A unique pointer to the merged KeyRecord of the key
     */
    static unique_ptr<KeyRecord> mergeToOneKey(
                                  const long base_key_index,
                                  const long number_of_keys_in_group,
                                  const double base_key_cdf,
                                  const dynamic& workload_config);
    /**
     * Split one key into multiple ones to scale up the number of keys
     * 
     * @param base_key_index The index of the key to split
     * @param number_of_keys_in_group The number of keys to split to
     * @param base_key_cdf The key CDF value of the previous key
     * @param workload_config The parsed JSON workload configuration
     * @return A vector of unique pointers to the splitted keys
     */
    static vector<unique_ptr<KeyRecord> > splitToMultipleKeys(
                                            const long base_key_index,
                                            const long number_of_keys_in_group,
                                            const double base_key_cdf,
                                            const dynamic& workload_config);
    /**
     * Return the index of the key given the random value and key CDF
     *
     * @param random_value A random double value in [0.0, 1.0]
     * @return The index of the first key with larger key CDF than random_value
     */
    long getRandomKeyIndex(double random_value);
    /**
     * Set the number of keys for statistics
     *
     * @param number_of_keys The number of keys in the workload
     */
    void setNumberOfKeys(long number_of_keys);
    /**
     * Set the average operation probability distribution function
     *
     * @param average_operation_pdf A map of the average operation PDF
     */
    void setAverageOperationPDF(
          map<string, double> average_operation_pdf);
    /**
     * Set the average object size for statistics
     *
     * @param average_object_size The average object size
     */
    void setAverageObjectSize(double average_object_size);
    /**
     * Print out the statistics of the workload
     */
    void printWorkloadStatistics();

    // A vector of records in the workload
    vector<unique_ptr<KeyRecord> > workload_records_;
    // Number of keys in the workload
    long number_of_keys_;
    // Map of the average operation type PDF
    map<string, double> average_operation_pdf_;
    // Average object size in the workload
    double average_object_size_;
};

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
