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
   * Guards an atomic flag, by trying to set it on construction with a fixed number of attempts,
   * spins, that defaults to zero. If setting the flag was successful, guard clears the flag on
   * destruction. The guard returns whether it was successful in setting the flag. It is also
   * possible to have another attempt at setting the flag.
   *
   * There is no locking or blocking involved, however, the guard can act as a spinlock.
   */
  class SpinGuard {
    enum class Set { NO, YES, DISOWNED };

    std::atomic_flag &flag_;
    Set set_;

    bool trySet(int spins) noexcept {
      if (set_ == Set::NO) {
        for (int i = 0; i < spins; ++i) {
          if (!flag_.test_and_set()) {
            set_ = Set::YES;
            return true;
          }
        }
      }
      return isSet();
    }

  public:
    SpinGuard(const SpinGuard &) = delete;
    SpinGuard(SpinGuard && moved) : flag_(moved.flag_), set_(moved.set_) {
      moved.set_ = Set::DISOWNED;
    }

    SpinGuard(std::atomic_flag &flag, int spins = 1) : flag_(flag), set_(Set::NO) {
      trySet(spins);
    }

    [[nodiscard]] bool isSet() const noexcept {
      return set_ == Set::YES;
    }

    [[nodiscard]] bool set(int spins = 1) noexcept {
      return trySet(spins);
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

    ~SpinGuard() {
      if (isSet()) {
        flag_.clear();
      }
    }
  };

  class GuardedFlag {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
  public:
    [[nodiscard]] SpinGuard guard(int spins = 1) { return SpinGuard(flag_, spins); }

  };

} // namespace simpledsp

#endif //SIMPLE_DSP_SPINGUARD_HPP
