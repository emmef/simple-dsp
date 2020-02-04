#ifndef SIMPLE_DSP_DSPRUNTIME_H
#define SIMPLE_DSP_DSPRUNTIME_H
/*
 * simple-dsp/dspruntime.h
 *
 * Added by michel on 2019-12-01
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

#include "attributes.h"
#include "guards.h"
#include "queue.h"
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdatomic.h>
#include <stdexcept>

namespace simpledsp {

class SampleRate {
  double rate_ = 0;

public:
  static constexpr double min = 1e-30;
  static constexpr double max = 1e+30;
  static double validated(double rate) {
    return rate == 0 ? 0 : std::clamp(rate, min, max);
  }

  SampleRate() {}
  explicit SampleRate(double rate) noexcept : rate_(validated(rate)) {}
  explicit SampleRate(const SampleRate &rate) noexcept = default;
  explicit SampleRate(SampleRate &&rate) noexcept = default;
  void assign(double rate) noexcept { rate_ = validated(rate); }
  SampleRate &operator=(double rate) noexcept {
    assign(rate);
    return *this;
  }
  operator double() const noexcept { return rate_; }
  explicit operator bool() const noexcept { return rate_ != 0; }
  double durationForFrameOfSize(size_t size) const noexcept {
    return rate_ == 0 ? std::numeric_limits<double>::infinity() : size / rate_;
  }
  bool isSet() const noexcept { return rate_ != 0; }
  double get() const noexcept { return operator double(); }
  bool operator==(double rate) const noexcept { return rate_ == rate; }
  bool operator==(const SampleRate &other) const noexcept {
    return rate_ == other.rate_;
  }
};

template <size_t MAXIMUM, class Subclass> class NumberOf {
  static_assert(MAXIMUM > 0, "Maximum \"number of\" MUST be positive.");
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
  static bool isValid(size_t number) { return number < max; }
  NumberOf() noexcept = default;
  NumberOf(size_t number) : number_(getValid(number)) {}

  void assign(const size_t &number) { number_ = getValid(number); }
  Subclass &operator=(const size_t &number) {
    assign(number);
    return *reinterpret_cast<Subclass>(this);
  }
  operator size_t() const noexcept { return number_; }
  explicit operator bool() const noexcept { return isSet(); }
  bool isSet() const noexcept { return number_ != 0; }
  size_t get() const noexcept { return operator size_t(); }
  bool operator==(size_t number) const noexcept { return number_ == number; }
  bool operator==(const NumberOf &number) const noexcept {
    return number_ == number.number_;
  }
};

class Ports : public NumberOf<16384, Ports> {
  using Super = class NumberOf<16384, Ports>;
  friend Super;

  static const std::string &invalidNumberMessage() {
    static std::string msg = "Ports: Value exceeds maximum number of " +
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
    static std::string msg = "Frames per buffer: exceeds maximum number of " +
                             std::to_string(Super::max) + " frames.";
    return msg;
  }

public:
  Frames() = default;
  Frames(size_t ports) : Super(ports) {}
};

class IOSpec {
  Ports inputs_;
  Ports outputs_;

public:
  IOSpec() = default;
  IOSpec(const IOSpec &) = default;
  IOSpec(IOSpec &&) = default;
  IOSpec(size_t inputs, size_t outputs) : inputs_(inputs), outputs_(outputs) {}

  bool isSet() const noexcept { return operator bool(); }
  operator bool() const noexcept { return inputs_ && outputs_; }
  Ports &inputs() noexcept { return inputs_; }
  Ports &outputs() noexcept { return outputs_; }

  IOSpec withInputs(size_t inputs) const { return {inputs, outputs_}; }

  IOSpec withOutputs(size_t outputs) const { return {inputs_, outputs}; }
};

class IOBase {
public:
  /**
   * @return the IOspec that the IO is based on.
   */
  sdsp_nodiscard virtual const IOSpec &ioSpec() const noexcept = 0;
};

class PhysicalContext {
  SampleRate rate_;
  Frames frames_;

public:
  PhysicalContext() = default;
  PhysicalContext(const PhysicalContext &other) noexcept = default;
  PhysicalContext(PhysicalContext &&other) noexcept = default;
  PhysicalContext(double rate, size_t frames) : rate_(rate), frames_(frames) {}
  bool isSet() const noexcept { return operator bool(); }
  explicit operator bool() const noexcept { return rate_.isSet() && frames_; }
  bool operator==(const PhysicalContext &other) const noexcept {
    return rate_ == other.rate_ && frames_ == other.frames_;
  }
  SampleRate &rate() noexcept { return rate_; }
  Frames &frames() noexcept { return frames_; }

  PhysicalContext withRate(float rate) { return {rate, frames_}; }
  PhysicalContext withFrames(size_t frames) { return {rate_, frames}; }
};

enum class RuntimeResult {
  SUCCESS,
  BUSY,
  FAILURE,
  NOOP,
  INCOMPATIBLE,
  NOT_RUNNING,
  ALREADY_INITIALIZED,
  NOT_INITIALIZED
};

template <class UserConfig> class RuntimeInfo {
  UserConfig userConfig_;
  IOSpec ioSpec_;
  PhysicalContext context_;

protected:
  RuntimeInfo(const UserConfig &config) : userConfig_(config){};
  void setPhysicalContext(const PhysicalContext &context) noexcept {
    context_ = context;
  }

  void setIOSpec(const IOSpec &ioSpec) noexcept { ioSpec_ = ioSpec; }

  void setUserConfig(const UserConfig &config) noexcept {
    userConfig_ = config;
  }

  bool equalRuntimeParameters(const PhysicalContext &context,
                              const UserConfig &config) const noexcept {
    return context == context_ && userConfig_->operator=(config);
  }

  bool equalParameters(const PhysicalContext &context, const UserConfig &config,
                       const IOSpec &ioSpec) const noexcept {
    return ioSpec == ioSpec_ && equalParameters(context, config);
  }

  bool equalParameters(const RuntimeInfo &other) const noexcept {
    return equalParameters(other.context_, other.userConfig_, other.ioSpec_);
  }

public:
  /**
   * @return the currently configured physical context.
   */
  sdsp_nodiscard const PhysicalContext &physicalContext() const noexcept {
    return context_;
  }

  /**
   * @return the currently set user configuration.
   */
  sdsp_nodiscard const UserConfig *userConfig() const noexcept {
    return *userConfig_;
  }

  /**
   * @return the currently configured IOSpec.
   */
  sdsp_nodiscard const IOSpec &ioSpec() const noexcept { return ioSpec_; }
};

template <class T> class SwitchedOwner {
  T *one_ = nullptr;
  T *two_ = nullptr;
  T *transport_ = nullptr;
  T *using_ = nullptr;
  GuardedFlag flag_;
  bool write_ = false;
  bool read_ = false;

  T *getTransport() const noexcept { return write_ ? one_ : two_; }

public:
  bool isInitialized() const noexcept { return two_; }

  template <typename... Args>
  RuntimeResult init(int tries, T *(*createWithParameters)(Args...),
                     Args... arguments) noexcept {
    auto guard = flag_.guard(tries, UseFence::YES);
    if (!guard.isSet()) {
      return RuntimeResult::BUSY;
    }
    if (two_) {
      return RuntimeResult::ALREADY_INITIALIZED;
    }
    two_ = createWithParameters(arguments...);
    one_ = createWithParameters(arguments...);
    if (two_) {
      if (!one_) {
        delete two_;
        two_ = nullptr;
      }
    } else if (one_) {
      delete one_;
      one_ = nullptr;
    }
    if (!two_) {
      return RuntimeResult::FAILURE;
    }
    write_ = read_ = false;
    transport_ = getTransport();
    write_ = !write_;
    return RuntimeResult::SUCCESS;
  }

  RuntimeResult clear() noexcept {
    if (!two_) {
      return RuntimeResult::NOOP;
    }
    delete two_;
    delete one_;
  }

  template <typename... Args>
  RuntimeResult writeParameters(int tries, bool (*setParameters)(T *, Args...),
                                Args... arguments) noexcept {
    auto guard = flag_.guard(tries, UseFence::YES);
    if (!guard.isSet()) {
      return RuntimeResult::BUSY;
    }
    if (write_ != read_) {
      return RuntimeResult::NOOP;
    }
    transport_ = getTransport();
    if (!transport_) {
      return RuntimeResult::NOT_INITIALIZED;
    }
    if (setParameters(transport_, arguments...)) {
      write_ = !write_;
      return RuntimeResult::SUCCESS;
    }
    return RuntimeResult::FAILURE;
  }

  template <typename... Args>
  RuntimeResult checkCompatible(int tries, bool (*check)(T *, Args...),
                                Args... arguments) noexcept {
    auto guard = flag_.guard(tries, UseFence::YES);
    if (!guard.isSet()) {
      return RuntimeResult::BUSY;
    }
    if (write_ != read_) {
      return RuntimeResult::NOOP;
    }
    transport_ = getTransport();
    if (!transport_) {
      return RuntimeResult::NOT_INITIALIZED;
    }
    if (check(transport_, arguments...)) {
      return RuntimeResult::SUCCESS;
    }
    return RuntimeResult::INCOMPATIBLE;
  }

  RuntimeResult read(T *&result) noexcept {
    auto guard = flag_.guard(1, UseFence::NO);
    MemoryFence fence;
    /*
     * Reading is lazy: at most this thread will not see that something has been
     * written.
     */
    if (write_ == read_) {
      result = using_;
      return RuntimeResult::NOOP;
    }
    using_ = transport_;
    result = using_;
    read_ = write_;
    return RuntimeResult::SUCCESS;
  }

  ~SwitchedOwner() {
    if (two_) {
      delete two_;
    }
    if (one_) {
      delete one_;
    }
  }
};

template <class UserConfig, class RuntimeConfig, class IO>
class Runtime : public RuntimeInfo<UserConfig> {
  enum class State { UNCONFIGURED, CONFIGURED, RUNNING };

  SwitchedOwner<RuntimeConfig> runtimeConfig_;
  std::atomic<State> state_ = State::UNCONFIGURED;

  static bool reconfigureRuntime(RuntimeConfig *config,
                                 const PhysicalContext &context,
                                 const UserConfig &userConfig) {
    return config && config->reconfigure(context, userConfig);
  }

  static bool checkRuntimeConfigurable(RuntimeConfig *config,
                                       const PhysicalContext &context,
                                       const UserConfig &userConfig) {
    return config && config->canReconfigureAtRuntime(context, userConfig);
  }

  static RuntimeConfig *createRuntimeConfig(const IOSpec &ioSpec,
                                            const PhysicalContext &context,
                                            const UserConfig &userConfig);

  /**
   * Actually processes data using the provided IO.
   * @see #process(IO&)
   */
  sdsp_nodiscard virtual bool doProcess(IO &io) noexcept = 0;

  /**
   * Applies an available new runtime configuration. This method is called from
   * the processCallback and might run in a realtime-context, which puts
   * restrictions on what the method is allowed to do.
   *
   * @return true if the configuration was successfully applied during runtime.
   */
  sdsp_nodiscard virtual RuntimeResult
  applyRuntimeConfig(RuntimeConfig *) noexcept = 0;

public:
  using RuntimeInfo<UserConfig>::ioSpec;
  using RuntimeInfo<UserConfig>::physicalContext;
  using RuntimeInfo<UserConfig>::userConfig;

  /**
   * Process the data, provided by the IO.
   * @param io The input and output data.
   * @return RuntimeResult::SUCCESS on success, RuntimeResult::INCOMPATIBLE if
   * the IO does not meet the currently configured specification and
   * RuntimeResult::FAILURE otherwise.
   */
  sdsp_nodiscard RuntimeResult process(IO &io) noexcept final {
    if (state_ != State::RUNNING) {
      return RuntimeResult::NOT_RUNNING;
    }
    RuntimeConfig *config = nullptr;
    if (runtimeConfig_.read(config) == RuntimeResult::SUCCESS) {
      RuntimeResult result = applyRuntimeConfig(config);
      if (result != RuntimeResult::SUCCESS) {
        return result;
      }
    }
    if (reinterpret_cast<const IOBase &>(io).ioSpec() == ioSpec()) {
      return process(io) ? RuntimeResult::SUCCESS : RuntimeResult::FAILURE;
    }
    return RuntimeResult::INCOMPATIBLE;
  }
};

//    template<class UserConfig, class RuntimeConfig, class IO>
//  class Runtime : public RuntimeInfo<UserConfig> {
//    static_assert(std::is_base_of<IOBase, IO>::value, "IO class does not have
//    IOBase as base.");
//
//    RuntimeConfig config[2];
//
//  protected:
//    /**
//     * Writes a new runtime configuration for changed context and / or
//     configuration.
//     * @see #writeRuntimeConfig()
//     */
//    sdsp_nodiscard virtual bool writeChangedRuntimeConfig(
//            const PhysicalContext &context,
//            const UserConfig &config,
//            RuntimeConfig &result) const = 0;
//
//    /**
//     * Changes the configuration for changed context and / or configuration.
//     * @see #reconfigure()
//     */
//    sdsp_nodiscard virtual bool changeConfiguration(
//            const PhysicalContext &context,
//            const UserConfig &config,
//            const IOSpec &ioSpec) const = 0;
//
//    /**
//     * Actually processes data using the provided IO.
//     * @see #process(IO&)
//     */
//    sdsp_nodiscard virtual bool doProcess(IO &io) noexcept = 0;
//
//    /**
//     * Applies an available new runtime configuration. This method is called
//     from the
//     * processCallback and might run in a realtime-context, which puts
//     restrictions on what
//     * the method is allowed to do.
//     *
//     * @return true if the configuration was successfully applied during
//     runtime.
//     */
//    sdsp_nodiscard virtual RuntimeResult applyRuntimeConfig() noexcept = 0;
//
//  public:
//
//    using RuntimeInfo<UserConfig>::ioSpec;
//    using RuntimeInfo<UserConfig>::physicalContext;
//    using RuntimeInfo<UserConfig>::userConfig;
//
//    /**
//     * Process the data, provided by the IO.
//     * @param io The input and output data.
//     * @return RuntimeResult::SUCCESS on success, RuntimeResult::INCOMPATIBLE
//     if the IO does not
//     * meet the currently configured specification and RuntimeResult::FAILURE
//     otherwise.
//     */
//    sdsp_nodiscard RuntimeResult process(IO &io) noexcept final {
//      if (reinterpret_cast<const IOBase &>(io).ioSpec() == ioSpec()) {
//        return process(io) ? RuntimeResult::SUCCESS : RuntimeResult::FAILURE;
//      }
//      return RuntimeResult::INCOMPATIBLE;
//    }
//
//    /**
//     * Returns whether the provided context is runtime-compatible with the
//     current.
//     *
//     * @param context The physical context to test.
//     * @return true if the provided context can be applied at runtime.
//     */
//    sdsp_nodiscard virtual bool canChangeRuntime(const PhysicalContext
//    &context) const noexcept = 0;
//
//    /**
//     * Returns whether the provided configuration is runtime-compatible wit
//     the current.
//     * @param config The configuration to test.
//     * @return true if the provided configuration can be applied at runtime.
//     */
//    sdsp_nodiscard virtual bool canChangeRuntime(const UserConfig &config)
//    const noexcept = 0;
//
//    /**
//     * Writes a new context to the result context. It is assumed that at least
//     one of context
//     * and configuration is different from the current one and that the checks
//     for runtime
//     * compatibility have already been done.
//     *
//     * A result is only written on a return value of RuntimeResult::SUCCESS.
//     *
//     * @param context The possibly changed physical context.
//     * @param config The possibly changed user configuration.
//     * @param result Contains the new runtime configuration, only on
//     RuntimeResult::SUCCESS.
//     * @return The result of the operation, which is RuntimeResult::NOOP if
//     nothing has changed,
//     * RuntimeResult::INCOMPATIBLE if the changes are incompatible,
//     RuntimeResult::SUCCESS if a
//     * new runtime compatible result was written to the result variable and
//     * RuntimeResult::FAILURE otherwise.
//     */
//    sdsp_nodiscard RuntimeResult writeRuntimeConfig(
//            const PhysicalContext &context,
//            const UserConfig &config) const final {
//      if (equalRuntimeParameters(context, &config)) {
//        return RuntimeResult::NOOP;
//      }
//      if (canChangeRuntime(config) && canChangeRuntime(context)) {
//        RuntimeConfig result;
//        if (writeChangedRuntimeConfig(context, config, result)) {
//          // manage a new version of the context
//          return RuntimeResult::SUCCESS;
//        }
//        return RuntimeResult::FAILURE;
//      }
//      return RuntimeResult::INCOMPATIBLE;
//    }
//
//    /**
//     * Returns whether the given combination of parameters are valid.
//     *
//     * @param context The physical context to test.
//     * @param config The user configuration to test.
//     * @param ioSpec The IO specification (inputs, outpus) to test.
//     * @return true if the combination of parameters is valid and a call to
//     reconfigure or the
//     * constructor would be successful.
//     */
//    sdsp_nodiscard bool isValidConfiguration(
//            const PhysicalContext &context,
//            const UserConfig &config,
//            const IOSpec &ioSpec) const noexcept = 0;
//
//    /**
//     * Reconfigures the runtime with a new set of parameters.
//     *
//     * This should not be done at runtime.
//     *
//     * @param context The possibly changed physical context.
//     * @param config The possibly changed user configuration.
//     * @param ioSpec The possibly changed IO specification.
//     *
//     * @return The result of the operation, which is RuntimeResult::NOOP if
//     nothing has changed,
//     * RuntimeResult::INCOMPATIBLE if the changes are incompatible,
//     RuntimeResult::SUCCESS if a
//     * new runtime compatible result was written to the result variable and
//     * RuntimeResult::FAILURE otherwise.
//     */
//    sdsp_nodiscard RuntimeResult reconfigure(
//            const PhysicalContext &context,
//            const UserConfig &config,
//            const IOSpec &ioSpec) final {
//      if (equalRuntimeParameters(context, config, ioSpec)) {
//        return RuntimeResult::NOOP;
//      }
//      if (isValidConfiguration(context, config, ioSpec)) {
//        if (changeConfiguration(context, config, ioSpec)) {
//          RuntimeInfo<UserConfig>::setIOSpec(ioSpec);
//          RuntimeInfo<UserConfig>::setPhysicalContext(context);
//          RuntimeInfo<UserConfig>::setUserConfig(config);
//          return RuntimeResult::SUCCESS;
//        }
//        return RuntimeResult::FAILURE;
//      }
//      return RuntimeResult::INCOMPATIBLE;
//    }
//  };
//
//  template<class UserConfig, class RuntimeConfig>
//  class Runtime {
//    PhysicalContext context_;
//  public:
//    sdsp_nodiscard bool writeConfig(const UserConfig &config, RuntimeConfig
//    &result) const {
//      return context_ && writeConfig(config, context_, result);
//    };
//    sdsp_nodiscard bool isValidContextChange(const PhysicalContext &context)
//    const {
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

#endif // SIMPLE_DSP_DSPRUNTIME_H
