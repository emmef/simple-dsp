#ifndef SIMPLE_DSP_SAMPLERATE_H
#define SIMPLE_DSP_SAMPLERATE_H
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
      struct HelperForSampleRateBase;

      template<typename freq>
      struct HelperForSampleRateBase<freq, true> {
        static constexpr freq min = 2 * std::numeric_limits<freq>::min();
        static constexpr freq max = std::numeric_limits<freq>::max();

        template <typename f>
        static constexpr freq getClamped(f value) {
          if (std::is_same<freq, f>::value) {
            return std::clamp(value, min, max);
          }
          else if (std::is_floating_point<f>::value) {
            return freq(std::clamp((long double)(value), (long double)(min), (long double)(max)));
          }
          else if (value > 0) {
            return freq(value);
          }
          return min;
        }
      };

      template<typename freq>
      struct HelperForSampleRateBase<freq, false> {
        static constexpr freq min = 2;
        static constexpr freq max = std::numeric_limits<freq>::max();

        template <typename f>
        static constexpr freq getClamped(f value) {
          if (std::is_same<freq, f>::value) {
            return std::clamp(value, min, max);
          }
          if (std::is_floating_point<f>::value) {
            return freq(std::clamp((long double)(value), (long double)(min), (long double)(max)));
          }
          else if (value > 2) {
            return freq(std::clamp(uint64_t(value), uint64_t(min), uint64_t(max)));
          }
          return 2;
        }
      };
    }

    template<typename freq>
    struct HelperForSampleRate :
            public HelperForSampleRateBase<freq, std::is_floating_point<freq>::value> {
      static_assert(std::is_integral<freq>::value || std::is_floating_point<freq>::value,
                      "Frequency type must be floating point or integral");
      using Base = HelperForSampleRateBase<freq, std::is_floating_point<freq>::value>;
      using Base::min;
      using Base::max;
      using frequency_type = freq;

      template <typename f>
      static constexpr freq clamped(f value) { return Base::template getClamped<f>(value); }

      template <typename f>
      static constexpr bool equals(freq value, f other) { return value == clamped(other); }

    };
  }

  template <typename freq = uint32_t>
  class SampleRate {
    using absolute = helper::HelperForSampleRate<freq>;

    freq rate = 1;
  public:
    using frequency_type = freq;

    template<typename otherFreq>
    static freq clamped(otherFreq value) { return absolute::clamped(value); }

    explicit SampleRate(freq f) : rate(clamped(f)) {}

    SampleRate(const SampleRate &f) = default;

    SampleRate(const SampleRate &&f) : rate(f.rate) {}

    SampleRate(SampleRate &&f) = default;

    operator const freq&() const { return rate; }

    operator freq&() { return rate; }

    sdsp_nodiscard freq nycquist() const { return rate / 2; }

    sdsp_nodiscard double relative(freq f) const { return f / rate; }

    sdsp_nodiscard double relativeNycquist(freq f) const { return f * 2 / rate; }

    sdsp_nodiscard static double angularSpeed(freq f) { return M_PI * 2 * f; }

    sdsp_nodiscard double relativeAngular(freq f) { return M_PI * 2 * relative(f); }

    template<typename otherFreq>
    SampleRate &operator = (otherFreq f) { rate = clamped(f); return *this; }

    SampleRate operator * (freq v) { return SampleRate(rate * v); }

    SampleRate operator + (freq v) { return SampleRate(rate * v); }

    SampleRate operator / (freq v) { return SampleRate(rate / clamped(v)); }

    SampleRate &operator *= (freq v) { rate = clamped(rate * v); return *this; }

    SampleRate &operator += (freq v) { rate = clamped(rate + v); return *this; }

    SampleRate &operator /= (freq v) {
      rate = clamped(rate / clamped(v)); return *this;
    }

    template<typename otherFreq>
    bool operator == (const otherFreq &other) const noexcept {
      return absolute::equals(other);
    }
  };



} // namespace simpledsp

#endif //SIMPLE_DSP_SAMPLERATE_H
