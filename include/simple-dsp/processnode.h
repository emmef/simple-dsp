#ifndef SIMPLE_DSP_PROCESSNODE_H
#define SIMPLE_DSP_PROCESSNODE_H
/*
 * simple-dsp/processnode.h
 *
 * Added by michel on 2019-11-26
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

#include <atomic>
#include <thread>
#include <type_traits>

#include <simple-dsp/core/attributes.h>
#include <simple-dsp/queue.h>

namespace simpledsp {

namespace process_traits {

/**
 * Runtime data that a processor uses in a processor callback. This data should
 * be initialised before it is used by the processor. After that, only the
 * processor should reconfigure the runtime data, using a RuntimeConfig
 * instance. This should already be based on useful information like sample-rate
 * and reconfiguring MUST be fast, cannot block, allocate memory or do anything
 * you should not do in a real-time process.
 *
 * @tparam RuntimeConfig The configuration that is used for initialisation and
 * reconfiguring.
 * @tparam Implementation The runtime data implemdetation class, configured with
 * a RuntimeConfig.
 */
template <class RuntimeConfig, class Implementation> class RuntimeDataTraits {
  enum class State { UNITIALIZED, BUSY, INITIALIZED };
  std::atomic<State> state_ = State::UNITIALIZED;

public:
  /**
   * Initialises the runtime data, based on the probided runtime-config.
   * This MUST not be called from the processing context.
   *
   * @param config The runtime configuration to use.
   * @return True if initialisation was succesful.
   */
  sdsp_nodiscard bool initConfig(const RuntimeConfig &config) {
    State expected = State::UNITIALIZED;
    if (!state_.compare_exchange_strong(expected, State::BUSY)) {
      return false;
    }
    bool result =
        reinterpret_cast<Implementation *>(this)->initConfigTrait(config);
    state_ = result ? State::INITIALIZED : State::UNITIALIZED;
    return result;
  }

  /**
   * Reconfigure this runtime data using the specified runtime config.
   *
   * This can and should be called from the processing context. If this is a
   * real-time context, the implementation cannot block, allocate and throw.
   *
   * @param config The new runtime configuration.
   * @return True if reconfiguration was successful.
   */
  sdsp_nodiscard bool changeConfig(const RuntimeConfig &config) noexcept {
    if (state_ == State::INITIALIZED) {
      return reinterpret_cast<Implementation *>(this)->changeConfigTrait(
          config);
    }
    return false;
  }

  /**
   * Destroys or cleans up the runtime data.
   *
   * This MUST not be called from the processing context.
   */
  void cleanup() noexcept {
    State initialized = State::INITIALIZED;
    if (state_.compare_exchange_strong(initialized, State::BUSY)) {
      reinterpret_cast<Implementation *>(this)->cleanupTrait();
      state_ = State::UNITIALIZED;
    }
  }
};

template <typename RuntimeConfig> class RuntimeConfigTraits {
  sdsp_nodiscard sdsp_force_inline bool
  operator==(const RuntimeConfigTraits &other) const noexcept {
    return reinterpret_cast<RuntimeConfig *>(this)->equals(other);
  }
  sdsp_nodiscard sdsp_force_inline float sampleRate() const noexcept {
    return reinterpret_cast<const RuntimeConfig *>(this)->sampleRateTrait();
  }
  sdsp_nodiscard sdsp_force_inline size_t bufferSize() const noexcept {
    return reinterpret_cast<const RuntimeConfig *>(this)->bufferSizeTrait();
  }
  sdsp_nodiscard sdsp_force_inline float secondsPerFrame() const noexcept {
    float bs = bufferSize();
    if (bs == 0) {
      return 1;
    }
    float rate = sampleRate();
    if (rate == 0) {
      return 1;
    }
    return bs / rate;
  }
};

} // namespace process_traits

/**
 * Describes a processing node.
 * @tparam IO The class that provides input and output buffers.
 * @tparam RuntimeData The runtime data used for processing.
 * @tparam RuntimeConfig
 * @tparam Message
 */
template <typename IO, typename RuntimeData, typename RuntimeConfig>
class ProcessingNode {
  static_assert(
      std::is_base_of<
          process_traits::RuntimeDataTraits<RuntimeConfig, RuntimeData>,
          RuntimeData>::value,
      "Type RuntimeData should implement process_traits::RuntimeDataTraits.");
  static_assert(
      std::is_base_of<process_traits::RuntimeConfigTraits<RuntimeConfig>,
                      RuntimeConfig>::value,
      "Type RuntimeConfig should implement "
      "process_traits::RuntimeConfigTraits");

  typedef process_traits::RuntimeDataTraits<RuntimeConfig, RuntimeData>
      *RuntimePtr;

  enum class CommandType { RECONFIGURE, STOP };
  enum class RuntimeState { NONE, READY, STOPPING, STOPPED };

  class RTGuard {
    QueueProducerConsumer<RuntimePtr> &queue_;
    RuntimePtr runtime_;

  public:
    RTGuard(QueueProducerConsumer<RuntimePtr> &queue, RuntimePtr runtime)
        : queue_(queue), runtime_(runtime) {}

    ~RTGuard() {
      if (runtime_) {
        queue_.put(runtime_);
      }
    }
  };

  GuardedFlag consistent_state_;
  std::atomic<RuntimeState> state_ = RuntimeState::NONE;
  std::atomic<size_t> frameCount_ = 0;
  std::atomic<RuntimePtr> runtime_ = nullptr;
  QueueProducerConsumer<RuntimePtr> runtimeIn_;
  QueueProducerConsumer<RuntimePtr> runtimeOut_;
  QueueProducerConsumer<RuntimeConfig> configs_;
  RuntimeConfig config_;

  void cleanupWaitingRuntimes() {
    RuntimePtr runtime;
    while (runtimeIn_.get(runtime) == QueueResult::SUCCESS) {
      if (runtime) {
        std::atomic_thread_fence(std::memory_order_acquire);
        delete runtime;
      }
    }
  }

  RuntimeState acknowledgeStoppingGetState() {
    /*
     * This is the sole non-consistent change to the state: all other access is
     * consistent as it is guarded by the consistent_state_ atomic flag.
     * The change is only effective FROM the state STOPPING. The STOPPING state
     * is only set, in a consistent context, by the stop() method.
     */
    RuntimeState state = RuntimeState::STOPPING;
    if (state_.compare_exchange_strong(state, RuntimeState::STOPPED)) {
      return RuntimeState::STOPPED;
    }
    return state;
  }

protected:
  sdsp_nodiscard virtual bool process(RuntimeData &data, IO &io) = 0;

public:
  enum class Result { SUCCESS, BUSY, SAME, FAILURE };

  ProcessingNode(size_t runtimes = 1, size_t configs = 2)
      : runtimeIn_(std::max(runtimes, 1LU)),
        runtimeOut_(std::max(runtimes, 1LU)), configs_(std::max(configs, 1LU)) {

  }

  ~ProcessingNode() {
    stop();
    RuntimePtr ptr = runtime_;
    if (runtime_.compare_exchange_strong(ptr, nullptr) && ptr != nullptr) {
      std::atomic_thread_fence(std::memory_order_acquire);
      delete ptr;
    }
  }

  template <typename... RuntimeConstructorArguments>
  Result setRuntime(const RuntimeConfig &config,
                    RuntimeConstructorArguments... arguments) {
    auto guard = consistent_state_.guard();
    if (!guard.isSet()) {
      return Result::BUSY;
    }
    RuntimeState state = state_;
    if (state != RuntimeState::NONE && state != RuntimeState::READY) {
      return Result::FAILURE;
    }
    if (state == RuntimeState::NONE) {
      cleanupWaitingRuntimes();
    }
    RuntimePtr newRuntime = new RuntimeData(arguments...);
    if (!newRuntime->initConfig(config)) {
      delete newRuntime;
      return Result::FAILURE;
    }
    config_ = config;
    std::atomic_thread_fence(std::memory_order_release);
    if (runtimeIn_.put(newRuntime != QueueResult::SUCCESS)) {
      delete newRuntime;
      return Result::FAILURE;
    }
    RuntimeState expected = RuntimeState::NONE;
    state_.compare_exchange_strong(expected, RuntimeState::READY);
    std::atomic_thread_fence(std::memory_order_release);

    return Result::SUCCESS;
  }

  Result setConfig(const RuntimeConfig &config) {
    auto guard = consistent_state_.guard();
    if (!guard.isSet()) {
      return Result::BUSY;
    }
    RuntimeState exp = RuntimeState::READY;
    if (!state_.compare_exchange_strong(RuntimeState::READY)) {
      return Result::FAILURE;
    }
    if (config_ == config) {
      return Result::SAME;
    }
    if (configs_.put(config) != QueueResult::SUCCESS) {
      return Result::BUSY;
    }
    config_ = config;
    std::atomic_thread_fence(std::memory_order_release);
    // flag operation stores
    return Result::SUCCESS;
  }

  void deactivateCallback() {
    RuntimeState state = acknowledgeStoppingGetState();
    if (state != RuntimeState::STOPPED) {
      stop();
    }
  }

  Result processCallback(IO &io) {

    std::atomic_thread_fence(std::memory_order_acquire);

    RuntimeState state = acknowledgeStoppingGetState();
    if (state != RuntimeState::READY) {
      return Result::FAILURE;
    }
    RuntimePtr rt = nullptr;
    RuntimePtr cleanup;
    if (runtimeIn_.get(rt) == QueueResult::SUCCESS && rt != nullptr) {
      cleanup = runtime_;
      runtime_ = rt;
    } else {
      cleanup = nullptr;
      rt = runtime_;
    }
    RTGuard guard(runtimeOut_, cleanup);
    if (rt == nullptr) {
      return Result::FAILURE;
    }

    RuntimeConfig config;
    if (configs_.get(config) == QueueResult::SUCCESS) {
      if (!rt->changeConfig(config)) {
        return Result::FAILURE;
      }
    }

    Result result = process(*reinterpret_cast<RuntimeData *>(rt), io)
                        ? Result::SUCCESS
                        : Result::FAILURE;

    frameCount_++;
    std::atomic_thread_fence(std::memory_order_release);
    return result;
  }

  void cleanupOldRuntimes() {
    RuntimePtr runtime;
    while (runtimeOut_.get(runtime) == QueueResult::SUCCESS) {
      std::atomic_thread_fence(std::memory_order_acquire);
      if (runtime) {
        delete runtime;
      }
    }
  }

  Result stop(long waitmillis = 100, int attempts = 100) {
    auto guard = consistent_state_.guard();
    if (!guard.isSet()) {
      return Result::BUSY;
    }
    RuntimeState exp = RuntimeState::READY;
    if (!state_.compare_exchange_strong(exp, RuntimeState::STOPPING)) {
      return Result::FAILURE;
    }
    if (waitmillis > 0) {
      int maxAttempts = std::clamp(attempts, 8, 100);
      for (int attempt = 0;
           state_ != RuntimeState::STOPPED && attempt < maxAttempts;
           attempt++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
      }
    }
    state_ = RuntimeState::STOPPED;
    cleanupOldRuntimes();
    cleanupWaitingRuntimes();

    return Result::SUCCESS;
  }

  Result resume() {
    auto guard = consistent_state_.guard();
    if (!guard.isSet()) {
      return Result::BUSY;
    }
    RuntimeState state = RuntimeState::STOPPED;
    if (!state_.compare_exchange_strong(state, state)) {
      return Result::FAILURE;
    }
    cleanupWaitingRuntimes();
    cleanupOldRuntimes();
    RuntimePtr runtime = runtime_;
    if (runtime && runtime->isConfigured()) {
      state_ = RuntimeState::READY;
      return Result::SUCCESS;
    }
    return Result::FAILURE;
  }
};
} // namespace simpledsp

#endif // SIMPLE_DSP_PROCESSNODE_H
