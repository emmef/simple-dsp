#ifndef SIMPLE_DSP_PERCEPTIVE_RMS_H
#define SIMPLE_DSP_PERCEPTIVE_RMS_H
/*
 * simple-dsp/perceptive-rms.h
 *
 * Added by michel on 2019-09-15
 * Copyright (C) 2015-2019 Michel Fleur.
 * Source https://github.com/emmef/simple-dsp
 * Email simple-dsp@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cmath>
#include <simple-dsp/addressing.h>
#include <simple-dsp/aligneddata.h>
#include <simple-dsp/delay.h>

namespace simpledsp {

  struct PerceptiveAverageTimes {
    double center;
    double peak;
    double powerRelation;
    double minimumDistance;

  } static constexpr DEFAULT_PERCEPTIVE_DATA {0.4, 0.00025, 0.25, 1.4};

  template<size_t MAXIMUM_WINDOWS>
  struct MovedRmsConfigData {
    static_assert(MAXIMUM_WINDOWS >= 3, "Need at least 3 RMS windows");

    void configure(size_t windows,
            double longestWindowSeconds,
            unsigned predictionPercentage,
            double maxPrediction) {
      if (windows > MAXIMUM_WINDOWS) {
        throw std::invalid_argument("Maximum number of windows exceeded");
      }
      predictionFactor_ = 0.01 * ::std::clamp(predictionPercentage, 0u, 100u);
      maxPrediction_ = ::std::clamp(maxPrediction, 0.0, 0.1);
      if (windows < 3) {
        throw std::invalid_argument("Maximum number of windows must be larger than 2");
      }
      windows_ = windows;
      if (longestWindowSeconds < perceptive_.center * sqrt(perceptive_.minimumDistance)) {
        longest_ = perceptive_.center;
      } else if (longestWindowSeconds < perceptive_.center * perceptive_.minimumDistance) {
        longest_ = perceptive_.center * perceptive_.minimumDistance;
      } else {
        longest_ = longestWindowSeconds;
      }
      size_t longCount;
      size_t shortCount;
      if (longest_ == perceptive_.center) {
        longCount = 0;
        shortCount = windows_ - 1;
      } else {
        double longPower = log(longest_ / perceptive_.center);
        double shortPower = log(perceptive_.center / perceptive_.peak);
        double powerFactor = (windows_ - 1) / (longPower + shortPower);
        longCount = longPower * powerFactor;
        shortCount = windows_ - 1 - longCount;
      }
      size_t i = 0;
      if (longCount > 0) {
        double factor = 1.0 / longCount;
        double base = perceptive_.center / longest_;
        for (; i < longCount; i++) {
          seconds_[i] = longest_ * pow(base, factor * i);
          scale_[i] = 1.0;
        }
      }
      seconds_[i] = perceptive_.center;
      scale_[i] = 1.0;
      i++;
      double factor = 1.0 / shortCount;
      double base = perceptive_.peak / perceptive_.center;
      for (size_t j = 1; j <= shortCount; i++, j++) {
        seconds_[i] = perceptive_.center * pow(base, factor * j);
        scale_[i] = pow(seconds_[i] / perceptive_.center, perceptive_.powerRelation);
      }
      if (maxPrediction_ == 0 || predictionFactor_ == 0) {
        for (size_t w = 0; w < windows; w++) {
          delay_[w] = 0;
        }
      } else {
        double maxP = 0;
        double minP = longest_;
        for (size_t w = 0; w < windows; w++) {
          double pred = seconds_[i] * predictionFactor_;
          maxP = ::std::max(maxP, pred);
          minP = ::std::min(minP, pred);
        }

        maxP = std::min(maxP, maxPrediction_);
        for (size_t w = 0; w < windows; w++) {
          double pred = seconds_[i] * predictionFactor_;
          if (pred > maxP) {
            delay_[w] = 0.0;
          } else {
            delay_[w] = maxP - pred;
          }
        }
        sampleDelay_ = maxP;
      }
    }

    PerceptiveAverageTimes getPerceptive() const { return perceptive_; }
    size_t getWindows() const { return windows_; }
    double getLongest() const { return longest_; }
    double getMaxPrediction() const { return maxPrediction_; }
    double getPredictionFactor() const { return predictionFactor_; }
    double getSampleDelay() const { return sampleDelay_; }
    double scale(size_t i) const { return scale_[Index::Method::index(i, windows_)]; }
    double delay(size_t i) const { return delay_[Index::Method::index(i, windows_)]; }
    double seconds(size_t i) const { return seconds_[Index::Method::index(i, windows_)]; }

  private:
    PerceptiveAverageTimes perceptive_ = DEFAULT_PERCEPTIVE_DATA;
    size_t windows_ = 0;
    double longest_ = 0;
    double predictionFactor_ = 0;
    double maxPrediction_ = 0;
    double sampleDelay_ = 0;
    double seconds_[MAXIMUM_WINDOWS];
    double delay_[MAXIMUM_WINDOWS];
    double scale_[MAXIMUM_WINDOWS];
  };

  template<size_t MAXIMUM_WINDOWS>
  struct MovingRmsRuntimeData {

  };

  template<typename T>
  struct BaseRmsMaximumSet {
    BaseRmsMaximumSet(size_t maximumSamples, size_t windows)
            :
            maxSamples(Size<T>::validGet(maximumSamples)),
            maxWindows(Size<size_t>::validSumGet(Size<size_t>::validProductGet(windows,
                    2,
                    ValidGet::RESULT), 1, ValidGet::RESULT),
                    data(new T[maxSamples]),
                    ptr(new size_t[maxWindows * 2 + 1])) { }

  private:
    size_t maxSamples;
    size_t maxWindows;
    size_t* ptr;
    T* data = 0;
    T* scale;
  };

} // namespace simpledsp

#endif //SIMPLE_DSP_PERCEPTIVE_RMS_H
