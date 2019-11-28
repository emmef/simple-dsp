#ifndef SIMPLE_DSP_TIMEOUT_H
#define SIMPLE_DSP_TIMEOUT_H
/*
 * simple-dsp/timeout.h
 *
 * Added by michel on 2019-11-28
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
#include <chrono>
#include <thread>
#include <simple-dsp/attributes.h>

namespace simpledsp {

  class TimeOut {
  public:
    virtual void start() noexcept = 0;
    sdsp_nodiscard virtual bool inTime() = 0;
    sdsp_nodiscard virtual bool timeOut() = 0;

    static TimeOut& never() {
      class TimeoutNever : public TimeOut {
        void start() noexcept override { }
        sdsp_nodiscard bool timeOut() override { return false; }
        sdsp_nodiscard bool inTime() override { return true; }
      } static instance;
      return instance;
    }

    static TimeOut& immediate() {
      class TimeoutImmediate : public TimeOut {
        void start() noexcept override { }
        sdsp_nodiscard bool timeOut() override { return true; }
        sdsp_nodiscard bool inTime() override { return false; }
      } static instance;
      return instance;
    }
  };

  template<
          class DURATION,
          class CLOCK = std::chrono::system_clock>
  class SimpleTimeOut : public TimeOut {
    using time_point = typename CLOCK::time_point;
    using duration = DURATION;
    time_point deadline;
    duration ticks;

  public:

    explicit SimpleTimeOut(long timeout) noexcept {
      ticks = duration(std::max(1L, timeout));
    }

    void start() noexcept override {
      deadline = std::chrono::system_clock::now() + ticks;
    }

    sdsp_nodiscard bool timeOut() override {
      return std::chrono::system_clock::now() > deadline;
    }

    sdsp_nodiscard bool inTime() override {
      return std::chrono::system_clock::now() <= deadline;
    }
  };

  template<
          class DURATION,
          class CLOCK = std::chrono::system_clock>
  class SlicedTimeout : public TimeOut {
    using time_point = typename CLOCK::time_point;
    using duration = DURATION;
    time_point deadline;
    time_point lastYield;
    duration slice;
    long ticks;

  public:
    explicit SlicedTimeout(long timeout, int slices) noexcept {
      ticks = std::max(1L, timeout);
      slice = duration(std::max(1L, ticks / slices));
    }

    void start() noexcept override {
      auto now = std::chrono::system_clock::now();
      deadline = now + duration(ticks);
      lastYield = now;
    }

    sdsp_nodiscard bool timeOut() override {
      updateOrSleep();
      return std::chrono::system_clock::now() > deadline;
    }

    sdsp_nodiscard bool inTime() override {
      updateOrSleep();
      return std::chrono::system_clock::now() <= deadline;
    }

  private:

    void updateOrSleep() {
      auto now = std::chrono::system_clock::now();
      if (now - lastYield >= slice) {
        lastYield = now;
        std::this_thread::yield();
      }
    }

  };

  using TimeOutNanos = SimpleTimeOut<std::chrono::nanoseconds>;
  using TimeOutMicros = SimpleTimeOut<std::chrono::microseconds>;
  using TimeOutMillis = SimpleTimeOut<std::chrono::milliseconds>;
  using TimeOutSeconds = SimpleTimeOut<std::chrono::seconds>;

  using TimeOutNanosSliced = SlicedTimeout<std::chrono::nanoseconds>;
  using TimeOutMicrosSliced = SlicedTimeout<std::chrono::microseconds>;
  using TimeOutMillisSliced = SlicedTimeout<std::chrono::milliseconds>;
  using TimeOutSecondsSliced = SlicedTimeout<std::chrono::seconds>;


} // namespace simpledsp

#endif //SIMPLE_DSP_TIMEOUT_H
