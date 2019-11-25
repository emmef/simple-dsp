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

  enum class VisibilityType {
    ATOMIC,
    FENCE
  };

  namespace queue_detail { namespace {
      template<typename T, VisibilityType visibilityType>
      class BaseVisibleValue;

      template <typename T>
      class BaseVisibleValue<T, VisibilityType::FENCE> {
        T v_;
      public:
        BaseVisibleValue(const T& value) : v_(value) {
          std::atomic_thread_fence(std::memory_order_release);
        }

        const T load() const noexcept {
          std::atomic_thread_fence(std::memory_order_acquire);
          return v_;
        }
        void store(const T &value) noexcept {
          v_ = value;
          std::atomic_thread_fence(std::memory_order_release);
        }
      };

      template <typename T>
      class BaseVisibleValue<T, VisibilityType::ATOMIC> {
        std::atomic<T> v_;
      public:
        BaseVisibleValue(const T& value) : v_(value) {
        }

        const T load() const noexcept {
          return v_;
        }
        void store(const T &value) noexcept {
          v_ = value;
        }
      };

      template <typename T>
      static constexpr VisibilityType deduceVisibilityType() {
        return std::atomic<T>::is_always_lock_free
               ? VisibilityType::ATOMIC
               : VisibilityType::FENCE;
      }

    }} // namespace anonymous and detail

  template <typename T>
  struct VisibleValue : public queue_detail::BaseVisibleValue<T, queue_detail::deduceVisibilityType<T>()> {
    using queue_detail::BaseVisibleValue<T, queue_detail::deduceVisibilityType<T>()>::load;
    using queue_detail::BaseVisibleValue<T, queue_detail::deduceVisibilityType<T>()>::store;
  };



} // namespace simpledsp

#endif //SIMPLE_DSP_SPINGUARD_HPP
