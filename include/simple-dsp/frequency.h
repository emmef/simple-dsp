#ifndef SIMPLE_DSP_FREQUENCY_H
#define SIMPLE_DSP_FREQUENCY_H
/*
 * simple-dsp/frequency.h
 *
 * Added by michel on 2019-10-13
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
#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>
#include <simple-dsp/algorithm.h>

namespace simpledsp {

  namespace helper {
    namespace {
      template<typename freq, bool fp>
      struct HelperForFrequencyRangeBase;

      template<typename freq>
      struct HelperForFrequencyRangeBase<freq, true> {
        static constexpr freq min = std::numeric_limits<freq>::min();
        static constexpr freq max = std::numeric_limits<freq>::max();
        static constexpr freq standard = 2 * M_PI;
      };

      template<typename freq>
      struct HelperForFrequencyRangeBase<freq, false> {
        static constexpr freq min = 1;
        static constexpr freq max = std::numeric_limits<freq>::max();
        static constexpr freq standard = 44100;
      };
    }

    template<typename freq>
    struct HelperForFrequencyRange :
            public HelperForFrequencyRangeBase<freq, std::is_floating_point<freq>::value> {

      using HelperForFrequencyRangeBase<freq, std::is_floating_point<freq>::value>::min;
      using HelperForFrequencyRangeBase<freq, std::is_floating_point<freq>::value>::max;
      using HelperForFrequencyRangeBase<freq, std::is_floating_point<freq>::value>::standard;

      static constexpr freq clamped(freq f) { return std::clamp(f, min, max); }

      static constexpr freq clampedMaximum(freq f) { return std::clamp(f, 2 * min, max); }

      static constexpr bool isValid(freq f) { return is_within(f, min, max); }

      static constexpr bool isValid(freq f, freq rangeMaximum) {
        return is_within(f, min, rangeMaximum);
      }

      static constexpr double angularSpeed(freq f) {
        return M_PI * 2 * f;
      }
    };
  }

  /**
   * Describes a frequency range between a low minimum and a sample frequency.
   *
   * It can be used to clamp frequencies between sample rate, Nycquist frequency. It can also
   * be used to calculate relativce frequency or angular speed.
   *
   * @tparam freq Type for frequency values.
   */
  template<typename freq>
  struct FrequencyRangeBase {
    using absolute = helper::HelperForFrequencyRange<freq>;

    FrequencyRangeBase(freq minimum, freq sampleFrequency) :
            sampleRate(absolute::clampedMaximum(sampleFrequency)),
            minimum(std::clamp(minimum, absolute::min, sampleRate / 2))
    {}

    explicit FrequencyRangeBase(freq sampleFrequency) : FrequencyRangeBase(absolute::min,
            sampleFrequency)
    {}

    FrequencyRangeBase() : sampleRate(absolute::standard), minimum(absolute::min)
    {}

    FrequencyRangeBase(const FrequencyRangeBase &source) :
            sampleRate(source.sampleRate), minimum(source.mininum)
    {}

    FrequencyRangeBase(FrequencyRangeBase &&source) noexcept :
            sampleRate(source.sampleRate), minimum(source.mininum)
    {}

    FrequencyRangeBase &operator = (const FrequencyRangeBase &source) {
      sampleRate = source.sampleRate;
      minimum = source.minimum;
      return *this;
    }

    sdsp_nodiscard freq getSampleFrequency() const { return sampleRate; }

    sdsp_nodiscard freq getNycquistFrequency() const { return sampleRate / 2; }

    sdsp_nodiscard freq clamp(freq f) const {
      return std::clamp(f, absolute::min, sampleRate);
    }

    sdsp_nodiscard freq clampToNycquist(freq f) const {
      return std::clamp(f, absolute::min, getNycquistFrequency());
    }

    sdsp_nodiscard freq relative(freq f) const {
      return clamp(f) / sampleRate;
    }

    sdsp_nodiscard freq relativeClampedToNycquist(freq f) const {
      return clampToNycquist(f) / sampleRate;
    }

    sdsp_nodiscard freq relativeToNycquist(freq f) const {
      return 2 * clampToNycquist(f) / sampleRate;
    }

    sdsp_nodiscard double relativeAngularSpeed(freq f) const {
      return absolute::angularSpeed(relativeClampedToNycquist(f));
    }

    sdsp_nodiscard bool setMinimum(freq newMinimum) {
      minimum = std::min(newMinimum, getNycquistFrequency());
      return minimum == newMinimum;
    }

    sdsp_nodiscard FrequencyRangeBase<freq> withMinimum(freq otherMinimum) const {
      return FrequencyRangeBase<freq>(std::min(otherMinimum, getNycquistFrequency()), sampleRate);
    }

    sdsp_nodiscard bool setSampleRate(freq newSampleRate) {
      sampleRate = std::max(std::min(absolute::max, newSampleRate), minimum * 2);
      return sampleRate == newSampleRate;
    }

    sdsp_nodiscard FrequencyRangeBase<freq> withSampleRate(freq otherSampleRate) const {
      return FrequencyRangeBase<freq>(minimum,
              std::max(std::min(absolute::max, otherSampleRate), minimum * 2));
    }

  private:
    freq sampleRate;
    freq minimum;
  };

  using FrequencyRangeFp = FrequencyRangeBase<double>;
  using FrequencyRangeInt = FrequencyRangeBase<unsigned long>;


} // namespace simpledsp

#endif //SIMPLE_DSP_FREQUENCY_H
