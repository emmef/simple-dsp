#ifndef SIMPLE_DSP_SAMPLERATE_H
#define SIMPLE_DSP_SAMPLERATE_H
/*
 * simple-dsp/frequency.h
 *
 * Added by michel on 2019-10-13
 * Copyright (C) 2015-2020 Michel Fleur.
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
#include <cstdint>
#include <limits>
#include <simple-dsp/core/bounds.h>
#include <type_traits>

namespace simpledsp {

namespace helper {
namespace {
template <typename freq, bool fp> struct HelperForSampleRateBase;

template <typename freq> struct HelperForSampleRateBase<freq, true> {
  static constexpr freq min = 2 * std::numeric_limits<freq>::min();
  static constexpr freq max = std::numeric_limits<freq>::max();

  template <typename f> static constexpr freq getClamped(f value) noexcept {
    if (std::is_same<freq, f>::value) {
      return Val::clamp(value, f(min), f(max));
    } else if (std::is_floating_point<f>::value) {
      return freq(Val::clamp((long double)(value), (long double)(min),
                             (long double)(max)));
    } else if (value > 0) {
      return freq(value);
    }
    return min;
  }
};

template <typename freq> struct HelperForSampleRateBase<freq, false> {
  static constexpr freq min = 2;
  static constexpr freq max = std::numeric_limits<freq>::max();

  template <typename f> static constexpr freq getClamped(f value) noexcept {
    if (std::is_same<freq, f>::value) {
      return Val::clamp(value, f(min), f(max));
    }
    if (std::is_floating_point<f>::value) {
      /* As the value of some integers, including max, cannot be represented
       * accurately by a floating point number, we cannot use a clamp with the
       * floating point representations.
       */
      if (value < (long double)(min)) {
        return min;
      }
      if (value > (long double)(max)) {
        return max;
      }
      return value;
    } else if (value > 2) {
      return freq(Val::clamp(uint64_t(value), uint64_t(min), uint64_t(max)));
    }
    return 2;
  }
};
} // namespace

template <typename freq>
struct HelperForSampleRate
    : public HelperForSampleRateBase<freq,
                                     std::is_floating_point<freq>::value> {
  static_assert(std::is_integral<freq>::value ||
                    std::is_floating_point<freq>::value,
                "Frequency type must be floating point or integral");
  using Base =
      HelperForSampleRateBase<freq, std::is_floating_point<freq>::value>;
  using Base::max;
  using Base::min;
  using frequency_type = freq;

  template <typename f> static constexpr freq clamped(f value) noexcept {
    return Base::template getClamped<f>(value);
  }

  template <typename f>
  static constexpr bool equals(freq value, f other) noexcept {
    freq f1 = other;
    f f2 = value;
    return value == f1 && other == f2;
  }
};
} // namespace helper

template <typename freq> class SampleRateBase {
  using absolute = helper::HelperForSampleRate<freq>;

  freq rate_ = 1;

public:
  using frequency_type = freq;

  template <typename otherFreq> static freq clamped(otherFreq value) {
    return absolute::clamped(value);
  }

  template <typename otherFreq>
  static freq clamped(const SampleRateBase<otherFreq> &value) {
    return absolute::clamped(value.rate());
  }

  /**
   * Returns the relative representation error for the given value if it would
   * be stored in the frequency type of this sample rate.
   *
   * @tparam otherFreq Type of the given value
   * @param other Value of the given value
   * @return A relative representation error that is 0 for exact representation.
   */
  template <typename otherFreq>
  static constexpr double representationError(otherFreq other) noexcept {
    freq f1 = other;
    double r1 = f1;
    double r2 = other;
    return r1 == r2 ? 0
                    : r2 == 0 ? std::numeric_limits<double>::infinity()
                              : fabs(r1 - r2) / fabs(r2);
  }

  explicit SampleRateBase(freq f) : rate_(clamped(f)) {}

  template <typename otherFreq>
  explicit SampleRateBase(otherFreq f) : rate_(clamped(f)) {}

  SampleRateBase(const SampleRateBase &f) = default;

  template <typename otherFreq>
  SampleRateBase(const SampleRateBase<otherFreq> &f)
      : SampleRateBase(f.rate()) {}

  SampleRateBase(const SampleRateBase &&f) : rate_(f.rate_) {}

  SampleRateBase(SampleRateBase &&f) = default;

  operator const freq &() const noexcept { return rate_; }

  operator freq &() noexcept { return rate_; }

  freq rate() const noexcept { return rate_; }

  sdsp_nodiscard freq nycquist() const noexcept { return rate_ / 2; }

  sdsp_nodiscard double relative(freq f) const noexcept { return f / rate_; }

  sdsp_nodiscard double relativeNycquist(freq f) const noexcept {
    return f * 2 / rate_;
  }

  sdsp_nodiscard static double angularSpeed(freq f) { return M_PI * 2 * f; }

  sdsp_nodiscard double relativeAngular(freq f) noexcept {
    return M_PI * 2 * relative(f);
  }

  template <typename otherFreq>
  SampleRateBase &operator=(otherFreq f) noexcept {
    rate_ = clamped(f);
    return *this;
  }

  SampleRateBase operator*(freq v) noexcept {
    return SampleRateBase(rate_ * v);
  }

  SampleRateBase operator+(freq v) noexcept {
    return SampleRateBase(rate_ * v);
  }

  SampleRateBase operator/(freq v) noexcept {
    return SampleRateBase(rate_ / clamped(v));
  }

  SampleRateBase &operator*=(freq v) noexcept {
    rate_ = clamped(rate_ * v);
    return *this;
  }

  SampleRateBase &operator+=(freq v) noexcept {
    rate_ = clamped(rate_ + v);
    return *this;
  }

  SampleRateBase &operator/=(freq v) noexcept {
    rate_ = clamped(rate_ / clamped(v));
    return *this;
  }

  template <typename otherFreq>
  bool operator==(const otherFreq &other) const noexcept {
    return absolute::equals(rate_, other);
  }

  template <typename otherFreq>
  bool operator==(const SampleRateBase<otherFreq> &other) const noexcept {
    return absolute::equals(rate_, otherFreq(other));
  }

  template <typename otherFreq>
  bool operator!=(const otherFreq &other) const noexcept {
    return !absolute::equals(rate_, other);
  }

  template <typename otherFreq>
  bool operator!=(const SampleRateBase<otherFreq> &other) const noexcept {
    return !absolute::equals(rate_, otherFreq(other));
  }
};

template <typename freq1, typename freq2>
bool operator==(freq1 f, const SampleRateBase<freq2> &r) noexcept {
  return r == f;
}

template <typename freq1, typename freq2>
bool operator!=(freq1 f, const SampleRateBase<freq2> &r) noexcept {
  return r != f;
}

using SampleRate = SampleRateBase<float>;
} // namespace simpledsp

#endif // SIMPLE_DSP_SAMPLERATE_H
