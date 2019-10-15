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
      };

      template<typename freq>
      struct HelperForSampleRateBase<freq, false> {
        static constexpr freq min = 2;
        static constexpr freq max = std::numeric_limits<freq>::max();
      };
    }

    template<typename freq>
    struct HelperForSampleRate :
            public HelperForSampleRateBase<freq, std::is_floating_point<freq>::value> {

      using HelperForSampleRateBase<freq, std::is_floating_point<freq>::value>::min;
      using HelperForSampleRateBase<freq, std::is_floating_point<freq>::value>::max;

      static constexpr freq clamped(freq f) { return std::clamp(f, min, max); }
    };
  }

  template <typename freq>
  class SampleRateBase {
    using absolute = helper::HelperForSampleRate<freq>;

    freq rate = 1;
  public:
    explicit SampleRateBase(freq f) : rate(absolute::clamped(f)) {}

    explicit SampleRateBase(const SampleRateBase &f) : rate(f.rate) {}

    SampleRateBase(SampleRateBase &&f) : rate(f.rate) {}

    operator const freq&() const { return rate; }

    operator freq&() { return rate; }

    sdsp_nodiscard freq nycquist() const { return rate / 2; }

    sdsp_nodiscard double relative(freq f) const { return f / rate; }

    sdsp_nodiscard double relativeNycquist(freq f) const { return f * 2 / rate; }

    sdsp_nodiscard static double angularSpeed(freq f) { return M_PI * 2 * f; }

    sdsp_nodiscard double relativeAngular(freq f) { return M_PI * 2 * relative(f); }

    SampleRateBase &operator = (freq f) { rate = absolute::clamped(f); return *this; }

    SampleRateBase &operator = (const SampleRateBase &f) { rate = f.rate; return *this; }

    SampleRateBase operator * (freq v) { return SampleRateBase(rate * v); }

    SampleRateBase operator + (freq v) { return SampleRateBase(rate * v); }

    SampleRateBase operator / (freq v) { return SampleRateBase(rate / absolute::clamped(v)); }

    SampleRateBase &operator *= (freq v) { rate = absolute::clamped(rate * v); return *this; }

    SampleRateBase &operator += (freq v) { rate = absolute::clamped(rate + v); return *this; }

    SampleRateBase &operator /= (freq v) {
      rate = absolute::clamped(rate / absolute::clamped(v)); return *this;
    }

  };

  using SampleRate = SampleRateBase<float>;

} // namespace simpledsp

#endif //SIMPLE_DSP_SAMPLERATE_H
