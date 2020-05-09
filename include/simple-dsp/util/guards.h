#ifndef SIMPLE_DSP_UTIL_GUARDS_HPP
#define SIMPLE_DSP_UTIL_GUARDS_HPP
/*
 * simple-dsp/util/guards.h
 *
 * Added by michel on 2019-11-21
 * Copyright (C) 2015-2020 Michel Fleur.
 * Source https://github.com/emmef/simplejack
 * Email simplejack@emmef.org
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
#include <simple-dsp/core/attributes.h>

namespace simpledsp {
template <typename Object>
static void deleteOnceNotNull(std::atomic<Object *> &atomic) {
  Object *value = atomic;
  if (value && atomic.compare_exchange_strong(value, nullptr)) {
    delete value;
  }
}

enum class UseFence { YES, NO };

/**
 * Guards an atomic flag, by trying to set it on construction. If setting the
 * flag was successful, guard clears the flag on destruction. The guard returns
 * whether it was successful in setting the flag. It is also possible to have
 * another attempt at setting the flag.
 *
 * There is no locking or blocking involved, however, the guard can act as a
 * spinlock.
 */
class FlagGuard {
  enum class Set { NO, DISOWNED, UNFENCED, FENCED };

  std::atomic_flag &flag_;
  Set set_;

  bool trySet(UseFence fence, int tries) noexcept {
    if (set_ == Set::NO) {
      for (int i = 0; i < tries; i++) {
        if (!flag_.test_and_set()) {
          if (fence == UseFence::YES) {
            std::atomic_thread_fence(std::memory_order_acquire);
            set_ = Set::FENCED;
          } else {
            set_ = Set::UNFENCED;
          }
          return true;
        }
      }
    }
    return isSet();
  }

public:
  FlagGuard(const FlagGuard &) = delete;
  FlagGuard(FlagGuard &&moved) : flag_(moved.flag_), set_(moved.set_) {
    moved.set_ = Set::DISOWNED;
  }

  FlagGuard(std::atomic_flag &flag, UseFence fence, int tries)
      : flag_(flag), set_(Set::NO) {
    trySet(fence, tries);
  }

  FlagGuard(std::atomic_flag &flag) : FlagGuard(flag, UseFence::YES, 1) {}

  [[nodiscard]] bool isSet() const noexcept {
    return set_ == Set::UNFENCED || set_ == Set::FENCED;
  }

  [[nodiscard]] bool set(UseFence fence = UseFence::NO) noexcept {
    return trySet(fence, 1);
  }

  [[nodiscard]] bool setWitTries(int tries,
                                 UseFence fence = UseFence::NO) noexcept {
    return trySet(fence, tries);
  }

  template <typename... Args>
  [[nodiscard]] bool call(void (*callable)(Args...), Args... args) {
    if (isSet()) {
      callable(args...);
      return true;
    }
    return false;
  }

  template <typename R, typename... Args>
  [[nodiscard]] bool apply(R &result, R (*function)(Args...), Args... args) {
    if (isSet()) {
      result = function(args...);
      return true;
    }
    return false;
  }

  ~FlagGuard() {
    if (isSet()) {
      if (set_ == Set::FENCED) {
        std::atomic_thread_fence(std::memory_order_release);
      }
      flag_.clear();
    }
  }
};

class GuardedFlag {
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

public:
  [[nodiscard]] FlagGuard guard(UseFence fence = UseFence::NO) {
    return FlagGuard(flag_, fence, 1);
  }
  [[nodiscard]] FlagGuard guard(int tries, UseFence fence = UseFence::NO) {
    return FlagGuard(flag_, fence, tries);
  }
};

/**
 * Ensures that all loads within the scope of this instance see all stores of
 * another instance's scope-exit in another thread. This is about memory
 * visibility on scope boundaries, not about synchronising blocks of code.
 */
class MemoryFence {

  static std::atomic<bool> & variable() {
    static std::atomic<bool> variable_;
    return variable_;
  }

public:
  MemoryFence() { acquire(); }
  ~MemoryFence() { release(); }

  /**
   * Ensures that all loads after acquire() in this thread, will see all stores
   * of another thread that happen before that calls release().
   */
  static void acquire() noexcept {
    variable().load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
  }

  /**
   * Ensures that all stores before release() in this thread, will be seen by
   * all loads of another thread that happen after that calls acquire().
   */
  static void release() noexcept {
    std::atomic_thread_fence(std::memory_order_release);
    variable().store(1, std::memory_order_relaxed);
  }
};

/**
 * A MemoryFence that only acts when it is the outer one in a thread.
 * There are architectures where a memory fence is almost a no-op, but on some
 * architectures it can be a heavy operation. Fences in nested code might incur
 * considerable burdens.
 */
class NestedMemoryFence {

  static int &level() {
    static thread_local int level_ = 0;
    return level_;
  }

public:
  NestedMemoryFence(const NestedMemoryFence &) = delete;
  NestedMemoryFence(NestedMemoryFence &&original) = delete;
  NestedMemoryFence() noexcept {
    if (level()++ == 0) {
      MemoryFence::acquire();
    }
  }

  ~NestedMemoryFence() noexcept {
    if (--level() == 0) {
      MemoryFence::release();
    }
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_UTIL_GUARDS_HPP
