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
#include <cstdint>
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
        static constexpr freq getClamped(f value) noexcept {
          if (std::is_same<freq, f>::value) {
            return std::clamp(value, f(min), f(max));
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
        static constexpr freq getClamped(f value) noexcept {
          if (std::is_same<freq, f>::value) {
            return std::clamp(value, f(min), f(max));
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
      static constexpr freq clamped(f value) noexcept { return Base::template getClamped<f>(value); }

      template <typename f>
      static constexpr bool equals(freq value, f other) noexcept {
        freq f1 = other;
        f f2 = value;
        return value == f1 && other == f2;
      }

    };
  }

  template <typename freq>
  class SampleRate {
    using absolute = helper::HelperForSampleRate<freq>;

    freq rate_ = 1;
  public:
    using frequency_type = freq;

    template<typename otherFreq>
    static freq clamped(otherFreq value) { return absolute::clamped(value); }

    template<typename otherFreq>
    static freq clamped(const SampleRate<otherFreq> &value) { return absolute::clamped(value.rate()); }

    explicit SampleRate(freq f) : rate_(clamped(f)) {}

    template<typename otherFreq>
    explicit SampleRate(otherFreq f) : rate_(clamped(f)) {}

    SampleRate(const SampleRate &f) = default;

    template<typename otherFreq>
    SampleRate(const SampleRate<otherFreq> &f) : SampleRate(f.rate()) {}

    SampleRate(const SampleRate &&f) : rate_(f.rate_) {}

    SampleRate(SampleRate &&f) = default;

    operator const freq&() const noexcept { return rate_; }

    operator freq&() noexcept { return rate_; }

    freq rate() const noexcept { return rate_; }

    sdsp_nodiscard freq nycquist() const noexcept { return rate_ / 2; }

    sdsp_nodiscard double relative(freq f) const noexcept { return f / rate_; }

    sdsp_nodiscard double relativeNycquist(freq f) const noexcept { return f * 2 / rate_; }

    sdsp_nodiscard static double angularSpeed(freq f) { return M_PI * 2 * f; }

    sdsp_nodiscard double relativeAngular(freq f) noexcept { return M_PI * 2 * relative(f); }

    template<typename otherFreq>
    SampleRate &operator = (otherFreq f) noexcept { rate_ = clamped(f); return *this; }

    SampleRate operator * (freq v) noexcept { return SampleRate(rate_ * v); }

    SampleRate operator + (freq v) noexcept { return SampleRate(rate_ * v); }

    SampleRate operator / (freq v) noexcept { return SampleRate(rate_ / clamped(v)); }

    SampleRate &operator *= (freq v) noexcept { rate_ = clamped(rate_ * v); return *this; }

    SampleRate &operator += (freq v) noexcept { rate_ = clamped(rate_ + v); return *this; }

    SampleRate &operator /= (freq v) noexcept {
      rate_ = clamped(rate_ / clamped(v)); return *this;
    }

    template<typename otherFreq>
    bool operator == (const otherFreq &other) const noexcept {
      return absolute::equals(rate_, other);
    }

    template<typename otherFreq>
    bool operator == (const SampleRate<otherFreq> &other) const noexcept {
      return absolute::equals(rate_, otherFreq(other));
    }

    template<typename otherFreq>
    bool operator != (const otherFreq &other) const noexcept {
      return !absolute::equals(rate_, other);
    }

    template<typename otherFreq>
    bool operator != (const SampleRate<otherFreq> &other) const noexcept {
      return !absolute::equals(rate_, otherFreq(other));
    }
  };

  template<typename freq1, typename freq2>
  bool operator == (freq1 f, const SampleRate<freq2> &r) noexcept {
    return r == f;
  }

  template<typename freq1, typename freq2>
  bool operator != (freq1 f, const SampleRate<freq2> &r) noexcept {
    return r != f;
  }

} // namespace simpledsp

#endif //SIMPLE_DSP_SAMPLERATE_H
