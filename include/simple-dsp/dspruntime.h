#ifndef SIMPLE_DSP_DSPRUNTIME_H
#define SIMPLE_DSP_DSPRUNTIME_H
/*
 * simple-dsp/dspruntime.h
 *
 * Added by michel on 2019-12-01
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
#include <cstddef>
#include <limits>
#include <stdexcept>
#include "attributes.h"

namespace simpledsp {

  class SampleRate {
    double rate_ = 0;
  public:
    static constexpr double min = 1e-30;
    static constexpr double max = 1e+30;
    static double validated(double rate) { return rate == 0 ? 0 : std::clamp(rate, min, max); }

    SampleRate() {}
    explicit SampleRate(double rate) noexcept : rate_(validated(rate)) {}
    explicit SampleRate(const SampleRate &rate) noexcept = default;
    explicit SampleRate(SampleRate &&rate) noexcept = default;
    void assign(double rate) noexcept {
      rate_ = validated(rate);
    }
    SampleRate &operator=(double rate) noexcept {
      assign(rate); return *this;
    }
    operator double() const noexcept {
      return rate_;
    }
    double durationForFrameOfSize(size_t size) const noexcept {
      return rate_ == 0 ? std::numeric_limits<double>::infinity() : size / rate_;
    }
    bool isSet() const noexcept {
      return rate_ != 0;
    }
    double get() const noexcept {
      return operator double();
    }
    bool operator == (double rate) const noexcept {
      return rate_ == rate;
    }
    bool operator == (const SampleRate &other) const noexcept {
      return rate_ == other.rate_;
    }
  };

  template<size_t MAXIMUM, class Subclass>
  class NumberOf {
    static_assert(
            MAXIMUM > 0,
            "Maximum \"number of\" MUST be positive.");
    size_t number_ = 0;

  protected:
    static size_t getValid(size_t number) {
      if (isValid(number)) {
        return number;
      }
      throw std::invalid_argument(Subclass::invalidNumberMessage());
    }
  public:
    static constexpr size_t max = MAXIMUM;
    static bool isValid(size_t number) {
      return number < max;
    }
    NumberOf() noexcept = default;
    NumberOf(size_t number) : number_(getValid(number)) { }

    void assign(const size_t &number) {
      number_ = getValid(number);
    }
    Subclass &operator = (const size_t &number) {
      assign(number);
      return *reinterpret_cast<Subclass>(this);
    }
    operator size_t() const noexcept {
      return number_;
    }
    bool isSet() const noexcept {
      return number_ != 0;
    }
    size_t get() const noexcept {
      return operator size_t();
    }
    bool operator == (size_t number) const noexcept {
      return number_ == number;
    }
    bool operator == (const NumberOf &number) const noexcept {
      return number_ == number.number_;
    }
  };

  class Ports : public NumberOf<16384, Ports> {
    using Super = class NumberOf<16384, Ports>;
    friend Super;

    static const std::string &invalidNumberMessage() {
      static std::string msg =
              "Ports: Value exceeds maximum number of " +
                      std::to_string(Super::max) + " ports.";
      return msg;
    }
  public:
    Ports() = default;
    Ports(size_t ports) : Super(ports) {}
  };

  class Frames : public NumberOf<INT32_MAX, Frames> {
    using Super = class NumberOf<INT32_MAX, Frames>;
    friend Super;

    static const std::string &invalidNumberMessage() {
      static std::string msg =
              "Frames per buffer: exceeds maximum number of " +
                      std::to_string(Super::max) + " frames.";
      return msg;
    }
  public:
    Frames() = default;
    Frames(size_t ports) : Super(ports) {}
  };

  class PhysicalContext {
    SampleRate rate_;
    Frames frames_;
    Ports inputs_;
    Ports outputs_;

  public:

    PhysicalContext() = default;
    PhysicalContext(const PhysicalContext &other) noexcept  = default;
    PhysicalContext(PhysicalContext &&other) noexcept = default;
    PhysicalContext(double rate, size_t frames, size_t inputs, size_t outputs) noexcept :
      rate_(rate), frames_(frames), inputs_(inputs), outputs_(outputs) {

    }
    bool isSet() const noexcept {
      return operator bool();
    }
    operator bool() const noexcept {
      return rate_.isSet() && frames_ && inputs_ && outputs_;
    }
    bool operator == (const PhysicalContext &other) const noexcept {
      return
            rate_ == other.rate_ && frames_ == other.frames_ &&
            inputs_ == other.inputs_ && outputs_ == other.outputs_;
    }
    SampleRate &sampleRate() noexcept { return rate_; }
    Ports &inputs() noexcept { return inputs_; }
    Ports &outputs() noexcept { return outputs_; }
    Frames &frames() noexcept { return frames_; }

    PhysicalContext withRate(float rate) {
      return {rate, frames_, inputs_, outputs_ };
    }

    PhysicalContext withFrames(size_t frames) {
      return {rate_, frames, inputs_, outputs_ };
    }

    PhysicalContext withInputs(size_t inputs) {
      return {rate_, frames_, inputs, outputs_ };
    }

    PhysicalContext withOutputs(size_t outputs) {
      return {rate_, frames_, inputs_, outputs };
    }
  };

//  template<class UserConfig, class RuntimeConfig>
//  class Runtime {
//    PhysicalContext context_;
//  public:
//    sdsp_nodiscard bool writeConfig(const UserConfig &config, RuntimeConfig &result) const {
//      return context_ && writeConfig(config, context_, result);
//    };
//    sdsp_nodiscard bool isValidContextChange(const PhysicalContext &context) const {
//      return cont
//    };
//    bool newConfig(const UserConfig &config, RuntimeConfig &result);
//  protected:
//    virtual bool writeConfig(
//            const UserConfig &config,
//            const PhysicalContext &context,
//            RuntimeConfig &result) const = 0;
//  };

} // namespace simpledsp

#endif //SIMPLE_DSP_DSPRUNTIME_H
