#ifndef SIMPLE_DSP_SPINGUARD_HPP
#define SIMPLE_DSP_SPINGUARD_HPP
/*
 * simple-dsp/spinguard.h
 *
 * Added by michel on 2019-11-21
 * Copyright (C) 2015-2019 Michel Fleur.
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
namespace simpledsp {
  template <typename Object>
  static void deleteOnceNotNull(std::atomic<Object*> &atomic) {
    Object *value = atomic;
    if (atomic.compare_exchange_strong(value, nullptr)) {
      if (value) {
        delete value;
      }
    }
  }

  /**
   * Guards an atomic flag, by trying to set it on construction. If setting the flag was successful,
   * guard clears the flag on destruction. The guard returns whether it was successful in setting
   * the flag. It is also possible to have another attempt at setting the flag.
   *
   * There is no locking or blocking involved, however, the guard can act as a spinlock.
   */
  class FlagGuard {
    enum class Set { NO, YES, DISOWNED };

    std::atomic_flag &flag_;
    Set set_;

    bool trySet() noexcept {
      if (set_ == Set::NO) {
        if (!flag_.test_and_set()) {
          set_ = Set::YES;
          return true;
        }
      }
      return isSet();
    }

  public:
    FlagGuard(const FlagGuard &) = delete;
    FlagGuard(FlagGuard && moved) : flag_(moved.flag_), set_(moved.set_) {
      moved.set_ = Set::DISOWNED;
    }

    FlagGuard(std::atomic_flag &flag) : flag_(flag), set_(Set::NO) {
      trySet();
    }

    [[nodiscard]] bool isSet() const noexcept {
      return set_ == Set::YES;
    }

    [[nodiscard]] bool set() noexcept {
      return trySet();
    }

    template<typename...Args>
    [[nodiscard]] bool call(void(*callable)(Args...), Args ...args) {
      if (isSet()) {
        callable(args...);
        return true;
      }
      return false;
    }

    template<typename R, typename...Args>
    [[nodiscard]] bool apply(R &result, R(*function)(Args...), Args ...args) {
      if (isSet()) {
        result = function(args...);
        return true;
      }
      return false;
    }

    ~FlagGuard() {
      if (isSet()) {
        flag_.clear();
      }
    }
  };

  class GuardedFlag {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
  public:
    [[nodiscard]] FlagGuard guard() { return FlagGuard(flag_); }

  };

  class MemoryFence {
    enum class Type { FORCED, RECURSIVE, DISCONNECTED };
    enum class Operation { INCREASE, DECREASE };
    static int setLevel(Operation operation) {
      /*
       * Normally, an atomic thread local makes no sense. But I have difficulty reading the C++
       * memory model. If a std::atomic_memory_fence(std::memory_order_acquire) ensures that all
       * the loads after this statement will read values that have been stored by another thread
       * before a std::atomic_memory_fence(std::memory_order_release) without explict loads and
       * stores surrounding those statements, then we can use a normal, non atomic integer.
       */
      static thread_local std::atomic<int> scopeRecursion_ = 0;

      if (operation == Operation::INCREASE) {
        return scopeRecursion_++;
      }
      return --scopeRecursion_;
    }
    Type type_;
    MemoryFence(Type type) : type_(type) {
      if (
              type_ == Type::FORCED ||
              (type_ == Type::RECURSIVE && setLevel(Operation::INCREASE) == 0)) {
        std::atomic_thread_fence(std::memory_order_acquire);
      }
    }
  public:

    MemoryFence(MemoryFence &&original) : type_(original.type_) {
      original.type_ = Type::DISCONNECTED;
    }

    static MemoryFence recursive() noexcept { return MemoryFence(Type::RECURSIVE); }
    static MemoryFence forced() { return MemoryFence(Type::FORCED); }

    ~MemoryFence() {
      if (
              type_ == Type::FORCED ||
              (type_ == Type::RECURSIVE && setLevel(Operation::DECREASE) == 0)) {
        std::atomic_thread_fence(std::memory_order_release);
      }
    }

  };

} // namespace simpledsp

#endif //SIMPLE_DSP_SPINGUARD_HPP
