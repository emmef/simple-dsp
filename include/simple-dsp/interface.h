#ifndef SIMPLE_DSP_INTERFACE_H
#define SIMPLE_DSP_INTERFACE_H
/*
 * simple-dsp/interface.h
 *
 * Added by michel on 2020-01-05
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

#include <cstddef>
#include <simple-dsp/addressing.h>
#include <simple-dsp/attributes.h>
#include <simple-dsp/samplerate.h>

namespace simpledsp {

class Interface {
  size_t inputs_;
  size_t outputs_;
  SampleRate sampleRate_;
  bool realTime_;
  size_t bufferSize_;
#if defined(SIMPLE_DSP_INTERFACE_SIZELIMIT) &&                                 \
    SIMPLE_DSP_INTERFACE_SIZELIMIT > 0
  using Size_ = Size<double, SIMPLE_DSP_INTERFACE_SIZELIMIT>;
#else
  using Size_ = Size<double>;
#endif

  static bool isValidCombination(size_t inputs, size_t outputs,
                                 size_t bufferSize) {
    size_t y =
        Size_::validProductGet(inputs, outputs, ValidGet::RESULT_OR_INVALID);
    if (Size_::isValid(y)) {
      y = Size_::validProductGet(y, bufferSize, ValidGet::RESULT_OR_INVALID);
      return Size_::isValid(y);
    }
    return false;
  }

  template <typename freq>
  Interface(size_t inputs, size_t outputs, SampleRateBase<freq> sampleRate,
            bool realTime, size_t bufferSize)
      : inputs_(inputs), outputs_(outputs), sampleRate_(sampleRate),
        realTime_(realTime), bufferSize_(bufferSize) {}

public:
  explicit Interface(const Interface &source) = default;
  explicit Interface(Interface &&source) = default;

  size_t inputs() const noexcept { return inputs_; }
  size_t outputs() const noexcept { return outputs_; }
  const SampleRate sampleRate() const noexcept { return sampleRate_; }
  bool realTime() const noexcept { return realTime_; }
  size_t bufferSize() const noexcept { return bufferSize_; }

  template <typename freq>
  static Interface of(size_t inputs, size_t outputs,
                      SampleRateBase<freq> sampleRate, bool realTime,
                      size_t bufferSize) {
    if (isValidCombination(inputs, outputs, bufferSize)) {
      return {inputs, outputs, sampleRate, realTime, bufferSize};
    }
    throw std::invalid_argument("Interface::of(): "
                                "Number of inputs, outputs, buffer size or "
                                "their combination is invalid.");
  }

  bool operator==(const Interface &other) {
    return inputs_ == other.inputs() && outputs_ == other.outputs() &&
           sampleRate_ == other.sampleRate() && realTime_ == other.realTime() &&
           bufferSize_ == other.bufferSize();
  }

  bool operator!=(const Interface &other) { return !operator==(other); }

  Interface withInputs(size_t inputs) const {
    if (isValidCombination(inputs, outputs_, bufferSize_)) {
      return {inputs, outputs_, sampleRate_, realTime_, bufferSize_};
    }
    throw std::invalid_argument(
        "Interface::withInputs(inputs): Invalid number of inputs or "
        "invalid in combination with other parameters.");
  }

  Interface withOutputs(size_t outputs) const {
    if (isValidCombination(inputs_, outputs, bufferSize_)) {
      return {inputs_, outputs, sampleRate_, realTime_, bufferSize_};
    }
    throw std::invalid_argument(
        "Interface::withOutputs(outputs): Invalid number of outputs or "
        "invalid in combination with other parameters.");
  }

  Interface withBufferSize(size_t bufferSize) const {
    if (isValidCombination(inputs_, outputs_, bufferSize)) {
      return {inputs_, outputs_, sampleRate_, realTime_, bufferSize};
    }
    throw std::invalid_argument(
        "Interface::withBufferSize(bufferSize): Invalid buffer size or "
        "invalid in combination with other parameters.");
  }

  template <typename freq>
  Interface withSampleRate(SampleRateBase<freq> sampleRate) const noexcept {
    return {inputs_, outputs_, sampleRate, realTime_, bufferSize_};
  }

  Interface withRealTime(bool realTime) const noexcept {
    return {inputs_, outputs_, sampleRate_, realTime, bufferSize_};
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_INTERFACE_H
