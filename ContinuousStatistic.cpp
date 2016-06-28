/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "treadmill/ContinuousStatistic.h"

#include "treadmill/RandomEngine.h"
#include "treadmill/Util.h"

#include <cmath>
#include <mutex>
#include <unordered_map>

#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

const std::map<double, const std::string> kQuantiles = {
  { 0.01, "p01" },
  { 0.05, "p05" },
  { 0.10, "p10" },
  { 0.15, "p15" },
  { 0.20, "p20" },
  { 0.50, "p50" },
  { 0.80, "p80" },
  { 0.85, "p85" },
  { 0.90, "p90" },
  { 0.95, "p95" },
  { 0.99, "p99" }
};

/**
 * Orchestrates synchronization of histogram inputs so all threads have
 * the same bins in their histogram. A thread passes in |proposed| and if
 * it the first thread to propose a set of inputs it will be used from
 * then on. The function returns a set of inputs that should be used
 * regardless.
 *
 */
HistogramInput synchronizeGlobalHistogramRange(
                    std::string name,
                    const HistogramInput& proposed) {
  static std::mutex global_mutex;
  static std::unordered_map<std::string, HistogramInput>
            proto_histogram_inputs;
  std::lock_guard<std::mutex> lock(global_mutex);

  HistogramInput accepted = proposed;
  auto it = proto_histogram_inputs.find(name);
  if (it == proto_histogram_inputs.end()) {
    proto_histogram_inputs[name] = proposed;
  } else {
    accepted = it->second;
  }

  return accepted;
}

void ContinuousStatistic::rebinHistogram(double target_max_value) {
  double min_value = this->histogram_->getMinBin();
  double max_value = this->histogram_->getMaxBin();

  double new_max_value = 0.0;
  if (target_max_value < 0.0) {
    double max_exceptional = 0;
    for (int i = 0; i < exceptional_index_; i++) {
      max_exceptional = std::max(max_exceptional, exceptional_values_[i]);
    }
    double pow = std::log2(max_exceptional);
    new_max_value = std::pow(2, ceil(pow));
  } else {
    new_max_value = target_max_value;
  }

  HistogramInput input(kNumberOfBins,
                       min_value,
                       new_max_value);
  std::unique_ptr<Histogram> new_histogram(new Histogram(input));
  new_histogram->insertSmallerHistogramSamples(this->histogram_);

  for (int i = 0; i < exceptional_index_; i++) {
    new_histogram->addSample(exceptional_values_[i]);
  }
  exceptional_index_ = 0;
  this->histogram_.swap(new_histogram);
}

void ContinuousStatistic::setHistogramBins() {
  double min_value = 0.0;
  double max_value = 1.0;
  if (!calibrationSamples_.empty()) {
    min_value = *std::min_element(calibrationSamples_.begin(),
                                  calibrationSamples_.end());
    max_value = *std::max_element(calibrationSamples_.begin(),
                                  calibrationSamples_.end());
  }
  HistogramInput input(kNumberOfBins,
                       min_value / 2.0,
                       max_value * 2.0);
  HistogramInput acceptedHistogram
      = synchronizeGlobalHistogramRange(
          this->getName(),
          input);
  histogram_.reset(new Histogram(acceptedHistogram));
}

/**
 * Add a sample to statistic
 *
 * @param value
 */
void ContinuousStatistic::addSample(double value) {
  if (histogram_ == nullptr) {
    if (warmupSamples_ < nWarmupSamples_) {
      warmupSamples_++;
      return;
    }
    if (calibrationSamples_.size() < nCalibrationSamples_) {
      calibrationSamples_.push_back(value);
      return;
    }
    if (calibrationSamples_.size() == nCalibrationSamples_) {
      this->setHistogramBins();
      // Set all stats back to 0 after calibration
      s0_ = 0;
      s1_ = 0.0;
      s2_ = 0.0;
      a_ = 0.0;
      q_ = 0.0;
      min_ = 0.0;
      max_ = 0.0;
      minSet_ = false;
      maxSet_ = false;
    }
  }
  if (value > this->histogram_->getMaxBin()) {
    exceptional_values_[exceptional_index_] = value;
    exceptional_index_++;
    if (exceptional_index_ == kExceptionalValues) {
      this->rebinHistogram();
    }
  } else {
    histogram_->addSample(value);
  }
  s0_ += 1.0;
  s1_ += value;
  s2_ += (value * value);
  double tempA = a_;
  a_ = a_ + (value - a_) / s0_;
  q_ = q_ + (value - tempA) * (value - a_);
  if (minSet_) {
    min_ = std::min(min_, value);
  } else {
    min_ = value;
    minSet_ = true;
  }
  if (maxSet_) {
    max_ = std::max(max_, value);
  } else {
    max_ = value;
    maxSet_ = true;
  }
}

double ContinuousStatistic::getAverage() const {
  if (s0_ == 0) {
    return 0;
  }
  return s1_ / (double) s0_;
}

double ContinuousStatistic::getStdDev() const {
  if (s0_ == 0) {
    return 0;
  }
  return sqrt(q_ / (s0_ - 1.0));
}

double ContinuousStatistic::getCV() const {
  return this->getStdDev() / this->getAverage();
}

/**
 * Estimate a quantile
 *
 * @param quantile
 */
double ContinuousStatistic::getQuantile(double quantile) {
  return histogram_->getQuantile(quantile);
}

double ContinuousStatistic::meanConfidence() const {
  double z = 1.96;
  double e = z * this->getStdDev() / sqrt(this->s0_);
  return e;
}

double ContinuousStatistic::quantileConfidence(double quantile) const {
  int nResamples = 100;
  ContinuousStatistic estimateStatistic("");
  for (int i = 0; i < nResamples; i++) {
    ContinuousStatistic resampled("");
    for (int j = 0; j < this->s0_; j++) {
      double randQuantile = RandomEngine::getDouble();
      double sample = this->histogram_->getQuantile(randQuantile);
      resampled.addSample(sample);
    }
    estimateStatistic.addSample(resampled.getAverage());
  }
  return estimateStatistic.meanConfidence();
}

/**
 * Print out all the statistic
 */
void ContinuousStatistic::printStatistic() const {
  if (!histogram_) {
    LOG(INFO) << "Did not collect enough samples";
    return;
  }
  LOG(INFO) << "N Samples: " << s0_;
  LOG(INFO) << "Average: " << this->getAverage()
            << " +/- " << meanConfidence();
  LOG(INFO) << "Std. Dev.: " << this->getStdDev();
  LOG(INFO) << "Cv.: " << this->getCV();
  LOG(INFO) << "Min: " << this->min_;
  LOG(INFO) << "Max: " << this->max_;
  for (auto p : kQuantiles) {
    LOG(INFO) << p.second << " Percentile: "
              << this->histogram_->getQuantile(p.first);
  //            << " +/- " << quantileConfidence(p.first);
  }
  LOG(INFO) << "Min Bin " << histogram_->getMinBin();
  LOG(INFO) << "Max Bin " << histogram_->getMaxBin();

}

folly::dynamic ContinuousStatistic::toDynamic() const {
  folly::dynamic map = folly::dynamic::object;
  map["n_samples"] = this->s0_;
  map["average"] = this->getAverage();
  map["std_dev"] = this->getStdDev();
  if (this->histogram_) {
    for (const auto& p : kQuantiles) {
      map[p.second] = this->histogram_->getQuantile(p.first);
    }
    map["histogram"] = this->histogram_->toDynamic();
  }
  return map;
}

std::unordered_map<std::string, int64_t>
    ContinuousStatistic::getCounters() const {
  std::unordered_map<std::string, int64_t> m;
  m[name_ + ".count"] = s0_;
  m[name_ + ".avg"] = getAverage();
  m[name_ + ".stddev"] = getStdDev();
  if (this->histogram_) {
    for (const auto& p : kQuantiles) {
      m[name_ + '.' + p.second] = histogram_->getQuantile(p.first);
    }
  }
  return m;

}

void ContinuousStatistic::combine(const Statistic& stat0) {
  const ContinuousStatistic& stat =
    dynamic_cast<const ContinuousStatistic&>(stat0);

  // Leveraging this:
  // http://en.wikipedia.org/wiki/
  // Algorithms_for_calculating_variance#Parallel_algorithm
  if (s0_ + stat.s0_ > 0) {
    if (s0_ <= 0.0) {
      this->a_ = stat.s0_;
      this->q_ = stat.q_;
    } else if (stat.s0_ <= 0.0) {
      // no-op
    } else {
    double delta = (stat.a_ - a_);
    double mean = a_ + delta * (stat.s0_ / (s0_ + stat.s0_));
    this->a_ = mean;
    this->q_ = q_ + stat.q_ + delta * delta * s0_ * stat.s0_ / (s0_ * stat.s0_);
    }
  }

  this->s0_ += stat.s0_;
  this->s1_ += stat.s1_;
  this->s2_ += stat.s2_;

  this->min_ = (this->minSet_ == true ? std::min(this->min_, stat.min_) :
                                        stat.min_);
  this->max_ = (this->maxSet_ == true ? std::max(this->max_, stat.max_) :
                                        stat.max_);
  if (stat.histogram_ == nullptr) {
    return;
  }

  // Rebin hitogram to make sure all the exceptional values are in the hitogram
  std::unique_ptr<ContinuousStatistic> stat_to_combine(
      new ContinuousStatistic(stat));
  if (stat_to_combine->exceptional_index_ != 0) {
    stat_to_combine->rebinHistogram();
  }

  if (this->histogram_ == nullptr) {
    this->histogram_.reset(new Histogram(*stat_to_combine->histogram_));
  } else {
    // Use the larger max_value for the combined histogram
    double new_max_value = std::max(this->histogram_->getMaxBin(),
                                    stat_to_combine->histogram_->getMaxBin());
    if (this->histogram_->getMaxBin() != new_max_value) {
      this->rebinHistogram(new_max_value);
    }
    if (stat_to_combine->histogram_->getMaxBin() != new_max_value) {
      stat_to_combine->rebinHistogram(new_max_value);
    }
    this->histogram_->combine(*stat_to_combine->histogram_);
  }
}

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
