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

#include "Workload.h"

#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

/**
 * Constructor for Workload
 */
Workload::Workload() {

}

/**
 * Generator method taking a set of parameters including number of keys,
 * operation type CDF and MIN/MAX object size. This method will create
 * a workload with uniformly distributed key popularity, and each key will
 * have the same operation CDF and a uniformly distributed object size
 *
 * @param number_of_keys The number of keys for the workload
 * @param operation_cdf The cumulative distribution function for the operation
 * @param min_object_size The minimal size of an object
 * @param max_object_size The maximal size of an object
 * @return A shared pointer to 
 */
shared_ptr<Workload> Workload::generateWorkloadByParameter(
                                const long number_of_keys,
                                const map<double, OperationType> operation_cdf,
                                const int min_object_size,
                                const int max_object_size) {
  // Create the workload object
  shared_ptr<Workload> workload(new Workload());
  
  double key_cdf = 0.0;
  double key_cdf_unit = 1.0 / number_of_keys;

  for (long i = 0; i < number_of_keys; i++) {
    // The key of the record in string
    string key = to_string(i);

    // The cumulative distribution function value of the key
    // We might want to change this if there is a performance issue
    if (i == number_of_keys - 1) {
      key_cdf = 1.0;
    } else {
      key_cdf += key_cdf_unit;
    }

    // The cumulative distribution function for the operation
    map<double, OperationType> operation_cdf_map = operation_cdf;

    // The cumulative distribution function for the object sizes
    double object_size_cdf = 0.0;
    double object_size_cdf_unit = 1.0 /
      (max_object_size - min_object_size + 1);
    // Fill in the object size CDF map
    map<double, int> object_size_cdf_map;
    for (int j = min_object_size; j < max_object_size; j++) {
      object_size_cdf += object_size_cdf_unit;
      object_size_cdf_map.insert(make_pair(object_size_cdf, j));
    }
    // Initialize the last element to get rid of the precision issue
    object_size_cdf_map.insert(make_pair(1.0, max_object_size));

    // Create the record and push it into the vector
    workload->workload_records_.push_back(
        unique_ptr<KeyRecord>(new KeyRecord(key, key_cdf, operation_cdf_map,
                                            object_size_cdf_map)));
  }

  // Set the statistics about the workload
  workload->setNumberOfKeys(number_of_keys);
  map<OperationType, double> average_operation_pdf;
  map<double, OperationType> operation_cdf_map = operation_cdf;
  double base_average_operation_cdf= 0.0;
  for (map<double, OperationType>::iterator i = operation_cdf_map.begin();
       i != operation_cdf_map.end(); i++) {
    average_operation_pdf.insert(
        make_pair(i->second, i->first - base_average_operation_cdf));
    base_average_operation_cdf = i->first;
  }
  workload->setAverageOperationPDF(average_operation_pdf);
  workload->setAverageObjectSize(
      (double) (min_object_size + max_object_size) / 2.0);
  // Print out the workload statistics
  workload->printWorkloadStatistics();

  return workload;
}

/**
 * Generator method taking a JSON configuration file which contains
 * workload characteristics, and scale up/down the number of keys to
 * the amount needed
 *
 * @param number_of_keys The number of keys for the workload
 * @param config_file_path The path to the JSON configuration file
 * @return A shared pointer to a Workload object
 */
shared_ptr<Workload> Workload::generateWorkloadByConfigFile(
                                const long number_of_keys,
                                const string& config_file_path) {
  // Read the JSON configuration file
  ifstream config_file;
  string string_buffer;
  config_file.exceptions(ifstream::failbit | ifstream::badbit);
  try {
    // Read the entire file into a string buffer
    config_file.open(config_file_path);
    string_buffer.assign(istreambuf_iterator<char>(config_file),
                         istreambuf_iterator<char>());
  } catch (ifstream::failure e) {
    LOG(FATAL) << "Fail to open / read / close the configuration file: "
               << config_file_path;
  }

  // Parse the JSON configuration file
  dynamic workload_config = parseJson(string_buffer);

  // Get the original number of keys
  long original_number_of_keys = workload_config.size();

  shared_ptr<Workload> workload;

  // Decide whether we need to scale it up or down
  if (original_number_of_keys >= number_of_keys) {
    workload = scaleDownNumberOfKeys(original_number_of_keys, number_of_keys,
                                     workload_config);
  } else {
    workload = scaleUpNumberOfKeys(original_number_of_keys, number_of_keys,
                                   workload_config);
  }

  // Set the statistics about the workload
  workload->setNumberOfKeys(number_of_keys);
  map<OperationType, double> average_operation_pdf;
  double average_object_size = 0.0;
  double base_key_cdf = 0.0;
  for (long i = 0; i < number_of_keys; i++) {
    map<double, OperationType> current_operation_cdf =
      workload->workload_records_[i]->operation_cdf();
    map<double, int> current_object_size_cdf =
      workload->workload_records_[i]->object_size_cdf();
    // Update average operation PDF
    double base_operation_cdf = 0.0;
    for (map<double, OperationType>::iterator j =
           current_operation_cdf.begin();
         j != current_operation_cdf.end(); j++) {
      if (average_operation_pdf.find(j->second) != 
            average_operation_pdf.end()) {
        average_operation_pdf[j->second] +=
          (j->first- base_operation_cdf) *
          (workload->workload_records_[i]->key_cdf() - base_key_cdf);
      } else {
        average_operation_pdf[j->second] =
          (j->first- base_operation_cdf) *
          (workload->workload_records_[i]->key_cdf() - base_key_cdf);
      }
      base_operation_cdf = j->first;
    }
    // Update average object size
    double base_object_size_cdf = 0.0;
    for (map<double, int>::iterator j = current_object_size_cdf.begin();
         j != current_object_size_cdf.end(); j++) {
      average_object_size += j->second *
        (j->first - base_object_size_cdf) *
        (workload->workload_records_[i]->key_cdf() - base_key_cdf);
      base_object_size_cdf = j->first;
    }
    // Update the base key CDF
    base_key_cdf = workload->workload_records_[i]->key_cdf();
  }
  workload->setAverageOperationPDF(average_operation_pdf);
  workload->setAverageObjectSize(average_object_size);
  // Print out the workload statistics
  workload->printWorkloadStatistics();

  return workload;
}


/**
 * Return the number of keys in the workload
 *
 * @return The number of keys in the workload
 */
long Workload::number_of_keys() {
  return this->number_of_keys_;
}

/**
 * Return a map to the average operation PDF
 *
 * @return A map to the average operation PDF
 */
map<OperationType, double> Workload::average_operation_pdf() {
  return this->average_operation_pdf_;
}

/**
 * Return the average object size in the workload
 *
 * @return The average object size
 */
double Workload::average_object_size() {
  return this->average_object_size_;
}

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
shared_ptr<Workload> Workload::scaleDownNumberOfKeys(
                                const long original_number_of_keys,
                                const long number_of_keys,
                                const dynamic& workload_config) {
  // Create the workload object
  shared_ptr<Workload> workload(new Workload());

  // Calculate the scaling factor and remainder
  long scaling_factor = original_number_of_keys / number_of_keys;
  long scaling_remainder = original_number_of_keys % number_of_keys;

  /**
   * Divide the original keys into number_of_keys groups, where the first
   * scaling_remainder groups have (scaling_factor + 1) keys and the rest
   * groups have scaling_factor keys
   */
  long base_key_index = 0;
  long number_of_keys_in_group = 0;
  double base_key_cdf = 0.0;
  for (long i = 0; i < number_of_keys; i++) {
    number_of_keys_in_group = (i < scaling_remainder) ? (scaling_factor + 1)
                                                      : scaling_factor;
    workload->workload_records_.push_back(
        mergeToOneKey(base_key_index, number_of_keys_in_group,
                      base_key_cdf, workload_config));
    // Update base key index and base key cdf
    base_key_index += number_of_keys_in_group;
    base_key_cdf = workload_config[base_key_index - 1]["key_cdf"].asDouble();
  }

  return workload;
}

/**
 * Scale up the number of keys when more keys are needed
 *
 * @param original_number_of_keys The original number of keys in the config
 *                                file
 * @param number_of_keys The number of keys needed
 * @param workload_config The parsed JSON workload configuration
 * @return A shared pointer to a Workload object
 */
shared_ptr<Workload> Workload::scaleUpNumberOfKeys(
                                const long original_number_of_keys,
                                const long number_of_keys,
                                const dynamic& workload_config) {
  // Create the workload object
  shared_ptr<Workload> workload(new Workload());

  // Calculate the scaling factor and remainder
  long scaling_factor = number_of_keys / original_number_of_keys;
  long scaling_remainder = number_of_keys % original_number_of_keys;

  /**
   * Divide the needed keys into original_number_of_keys groups, where the
   * last scaling_remainder groups have (scaling_factor + 1) keys and the rest
   * groups have scaling_factor keys
   */
  long number_of_keys_in_group = 0;
  double base_key_cdf = 0.0;
  for (long i = 0; i < original_number_of_keys; i++) {
    number_of_keys_in_group =
      (i < original_number_of_keys - scaling_remainder) ? scaling_factor
                                                        : (scaling_factor + 1);
    vector<unique_ptr<KeyRecord> > splitted_keys =
      splitToMultipleKeys(i, number_of_keys_in_group, base_key_cdf,
                          workload_config);
    for (auto& item : splitted_keys) {
      workload->workload_records_.push_back(move(item));
    }
    // Update base key cdf
    base_key_cdf = workload_config[i]["key_cdf"].asDouble();
  }

  return workload;
}

/**
 * Merge multiple keys into one to scale down the number of keys
 *
 * @param base_key_index The index of the first key to merge
 * @param number_of_keys_in_group The number of keys to merge
 * @param base_key_cdf The key CDF value of the previous key
 * @param workload_config The parsed JSON workload configuration
 * @return A unique pointer to the merged KeyRecord of the key
 */
unique_ptr<KeyRecord> Workload::mergeToOneKey(
                                  const long base_key_index,
                                  const long number_of_keys_in_group,
                                  const double base_key_cdf,
                                  const dynamic& workload_config) {
  // Use the key of the first key in the group as the new key
  string key = workload_config[base_key_index]["key"].asString().toStdString();

  // The key CDF value for the merged key
  double merged_key_cdf =
    workload_config[base_key_index + number_of_keys_in_group - 1]["key_cdf"]
    .asDouble();

  // Local base key CDF value
  double local_base_key_cdf = base_key_cdf;
  // The range that the new key covers
  double key_cdf_range = merged_key_cdf - local_base_key_cdf;
  // Operation PDF map
  map<OperationType, double> operation_pdf;
  // Operation CDF map
  map<double, OperationType> operation_cdf;
  // Object size PDF map
  map<int, double> object_size_pdf;
  // Object size CDF map
  map<double, int> object_size_cdf;
  for (long i = base_key_index; i < base_key_index + number_of_keys_in_group;
       i++) {
    // Operation PDF
    double operation_cdf_base = 0.0;
    for (auto& operation_array : workload_config[i]["operation_cdf"]) {
      OperationType operation_type =
        kOperationTypeMap[operation_array[1].asString().toStdString()];
      if (operation_pdf.find(operation_type) != operation_pdf.end()) {
        operation_pdf[operation_type] +=
          (operation_array[0].asDouble() - operation_cdf_base) *
          (workload_config[i]["key_cdf"].asDouble() - local_base_key_cdf) /
          key_cdf_range;
      } else {
        operation_pdf[operation_type] =
          (operation_array[0].asDouble() - operation_cdf_base) *
          (workload_config[i]["key_cdf"].asDouble() - local_base_key_cdf) /
          key_cdf_range;
      }
      operation_cdf_base = operation_array[0].asDouble();
    }
    // Object size PDF
    double object_size_cdf_base = 0.0;
    for (auto& object_size_array : workload_config[i]["object_size_cdf"]) {
      int object_size = object_size_array[1].asInt();
      if (object_size_pdf.find(object_size) != object_size_pdf.end()) {
        object_size_pdf[object_size] +=
          (object_size_array[0].asDouble() - object_size_cdf_base) *
          (workload_config[i]["key_cdf"].asDouble() - local_base_key_cdf) /
          key_cdf_range;
      } else {
        object_size_pdf[object_size] =
          (object_size_array[0].asDouble() - object_size_cdf_base) *
          (workload_config[i]["key_cdf"].asDouble() - local_base_key_cdf) /
          key_cdf_range;
      }
      object_size_cdf_base = object_size_array[0].asDouble();
    }
    local_base_key_cdf = workload_config[i]["key_cdf"].asDouble();
  }
  
  // Turn operation PDF map to CDF map
  double operation_cdf_base = 0.0;
  for (map<OperationType, double>::iterator i = operation_pdf.begin();
       i != operation_pdf.end(); i++) {
    operation_cdf[i->second + operation_cdf_base] = i->first;
    operation_cdf_base = i->second;
  }

  // Turn object size PDF map to CDF map
  double object_size_cdf_base = 0.0;
  for (map<int, double>::iterator i = object_size_pdf.begin();
       i != object_size_pdf.end(); i++) {
    object_size_cdf[i->second + object_size_cdf_base] = i->first;
    object_size_cdf_base = i->second;
  }

  // Create the KeyRecord object for the new key
  unique_ptr<KeyRecord> record(new KeyRecord(key, merged_key_cdf,
                                             operation_cdf, object_size_cdf));
  return move(record);
}

/**
 * Split one key into multiple ones to scale up the number of keys
 * 
 * @param base_key_index The index of the key to split
 * @param number_of_keys_in_group The number of keys to split to
 * @param base_key_cdf The key CDF value of the previous key
 * @param workload_config The parsed JSON workload configuration
 * @return A vector of unique pointers to the splitted keys
 */
vector<unique_ptr<KeyRecord> > Workload::splitToMultipleKeys(
                                        const long base_key_index,
                                        const long number_of_keys_in_group,
                                        const double base_key_cdf,
                                        const dynamic& workload_config) {
  // A vector of unique pointers to the splitted keys
  vector<unique_ptr<KeyRecord> > record;

  // Key CDF value unit
  double key_cdf_unit =
    (workload_config[base_key_index]["key_cdf"].asDouble() - base_key_cdf) /
    number_of_keys_in_group;

  for (long i = 0; i < number_of_keys_in_group; i++) {
    // Append an integer to the end of the original key as the new key
    stringstream sstm;
    sstm << workload_config[base_key_index]["key"].asString().toStdString()
         << i;
    string key = sstm.str();

    // Uniformly divide the original CDF to number_of_keys_in_group slices
    double splitted_key_cdf = base_key_cdf + (i + 1) * key_cdf_unit;

    // Operation CDF map
    map<double, OperationType> operation_cdf;
    for (auto& operation_array : workload_config[i]["operation_cdf"]) {
      OperationType operation_type =
        kOperationTypeMap[operation_array[1].asString().toStdString()];
      operation_cdf[operation_array[0].asDouble()] = operation_type;
    }

    // Object size CDF map
    map<double, int> object_size_cdf;
    for (auto& object_size_array : workload_config[i]["object_size_cdf"]) {
      object_size_cdf[object_size_array[0].asDouble()] = 
        object_size_array[1].asInt();
    }

    // Create the KeyRecord object for the new key and push it into the vector
    record.push_back(
        unique_ptr<KeyRecord>(new KeyRecord(key, splitted_key_cdf,
                                            operation_cdf, object_size_cdf)));
  }

  return move(record);
}

/**
 * Set the number of keys for statistics
 *
 * @param number_of_keys The number of keys in the workload
 */
void Workload::setNumberOfKeys(long number_of_keys) {
  this->number_of_keys_ = number_of_keys;
}
/**
 * Set the average operation probability distribution function
 *
 * @param average_operation_pdf A map of the average operation PDF
 */
void Workload::setAverageOperationPDF(
                map<OperationType, double> average_operation_pdf) {
  this->average_operation_pdf_ = average_operation_pdf;
}

/**
 * Set the average object size for statistics
 *
 * @param average_object_size The average object size
 */
void Workload::setAverageObjectSize(double average_object_size) {
  this->average_object_size_ = average_object_size;
}

/**
 * Print out the statistics of the workload
 */
void Workload::printWorkloadStatistics() {
  double working_set_size = this->number_of_keys_ * this->average_object_size_;
  // Statistics about the workload
  LOG(INFO) << "Workload Statistics:";
  LOG(INFO) << "\t- " << "Number of Keys: " << this->number_of_keys_;
  for (map<string, OperationType>::iterator i = kOperationTypeMap.begin();
       i != kOperationTypeMap.end(); i++) {
    LOG(INFO) << "\t- " << "Portion of " << i->first << "Operations: "
              << this->average_operation_pdf_[i->second];
  }
  LOG(INFO) << "\t- " << "Average Object Size: " << this->average_object_size_;
  LOG(INFO) << "\t- " << "Total Working Set Size: " << working_set_size;
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
