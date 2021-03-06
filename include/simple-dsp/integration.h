#ifndef SIMPLE_DSP_INTEGRATION_H
#define SIMPLE_DSP_INTEGRATION_H
/*
 * simple-dsp/integration.h
 *
 * Added by michel on 2019-10-10
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
#include <limits>
#include <type_traits>
#include <simple-dsp/core/bounds.h>

namespace simpledsp {

/**
 * Describes integration system with a characteristic "RC" time in
 * samples and a unity scale. This translates into an IIR system with
 * two multipliers that can be implemented as follows:
 *
 *     y[n] = history_multiplier * y[n - 1] + input_multiplier * x[n]
 *
 * where x[n] and y[n] are respectively the inputs and outputs for sample
 * number n.
 *
 * The multipliers are dimensioned such that the total sums to infinity of the
 * input and output signals are equal.
 *
 * @tparam T type of sample.
 */
template <typename T> struct IntegrationMulipliers {
  static_assert(std::is_floating_point_v<T>,
                "Value type must be floating-point");

  /**
   * Bounds for multipliers where the calculation becomes unstable or too
   * inaccurate.
   */
  static constexpr T maxMultiplier = 1.0 - std::numeric_limits<T>::epsilon();
  static constexpr T minMultipier = uncheckedOtherMultiplier(maxMultiplier);
  /**
   * Bounds for maximm number of samples and the minimum number of samples where
   * the unity gain cannot be guanranteed: the number of samples will be set to
   * zero for these casses.
   *
   * The actual sum of an integrated impulse response will already have about a
   * promille of inaccuracy for about an eighth of the maximum number of
   * samples. Actual integration is very accurate for the whole range of sample
   * numbers.
   */
  static const T maxSamples() {
    static const T result =
        uncheckedSamplesFromHistoryMultiplier(maxMultiplier);
    return result;
  }

  static const T minSamples() {
    static const T result = 1.0 / maxSamples();
    return result;
  }

  static inline T historyMultiplier(T samples) noexcept {
    return samples < minSamples()
               ? 0
               : std::exp(-1.0 / (minimum(samples, maxSamples())));
  }

  static inline T inputMultiplier(T samples) noexcept {
    return uncheckedOtherMultiplier(historyMultiplier(samples));
  }

  static constexpr T uncheckedOtherMultiplier(T multiplier) noexcept {
    return 1.0 - multiplier;
  }

  static inline T samplesFromHistoryMultiplier(T historyMultiplier) noexcept {
    return historyMultiplier < minMultipier
               ? 0
               : historyMultiplier > maxMultiplier
                     ? maxSamples()
                     : -1.0 / log(historyMultiplier);
  }

  static inline T samplesFromInputMultiplier(T inputMultiplier) noexcept {
    return inputMultiplier > maxMultiplier
               ? 0
               : inputMultiplier < minMultipier
                     ? maxSamples()
                     : -1.0 / log(uncheckedOtherMultiplier(inputMultiplier));
  }

  static constexpr T getIntegrated(T input, T inputMultiplier, T previousOutput,
                                   T outputMultiplier) noexcept {
    return input * inputMultiplier + previousOutput * outputMultiplier;
  }

  static void integrate(T input, T inputMultiplier, T &output,
                        T outputMultiplier) noexcept {
    output *= outputMultiplier;
    output += input * inputMultiplier;
  }

  static inline T
  uncheckedSamplesFromHistoryMultiplier(T historyMultiplier) noexcept {
    return -1.0 / log(historyMultiplier);
  }

  static inline T
  uncheckedSamplesFromInputMultiplier(T inputMultiplier) noexcept {
    return -1.0 / log(uncheckedOtherMultiplier(inputMultiplier));
  }
};

template <typename T> class IntegrationCoefficients {
  using Multipliers = IntegrationMulipliers<T>;
  T historyMultiplier = 0;
  T inputMultiplier = 1.0;

public:
  IntegrationCoefficients() = default;

  IntegrationCoefficients(T samples)
      : historyMultiplier(Multipliers::historyMultiplier(samples)),
        inputMultiplier(
            Multipliers::uncheckedOtherMultiplier(historyMultiplier)) {}

  T history() const noexcept { return historyMultiplier; }
  T input() const noexcept { return inputMultiplier; }
  T samples() const noexcept {
    return Multipliers::samplesFromHistoryMultiplier(historyMultiplier);
  }

  void setSamples(T samples) noexcept {
    historyMultiplier = Multipliers::historyMultiplier(samples);
    inputMultiplier = Multipliers::uncheckedOtherMultiplier(historyMultiplier);
  }

  void setSecondsWithRate(T sampleRate, T seconds) noexcept {
    return setSamples(sampleRate * seconds);
  }

  T getIntegrated(T input, T previousOutput) noexcept {
    return Multipliers::getIntegrated(input, inputMultiplier, previousOutput,
                                      historyMultiplier);
  }

  void integrate(T input, T &output) noexcept {
    Multipliers::integrate(input, inputMultiplier, output, historyMultiplier);
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_INTEGRATION_H
