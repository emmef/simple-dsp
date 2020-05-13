#ifndef SIMPLE_DSP_CORE_DENORMAL_H
#define SIMPLE_DSP_CORE_DENORMAL_H
/*
 * simple-dsp/core/denormal.h
 *
 * Added by michel on 2019-09-10
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

#include <cinttypes>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace simpledsp {

template <typename T> struct FTZFor {
  static_assert(std::is_floating_point_v<T>);

  static constexpr bool is_iec559 = std::numeric_limits<T>::is_iec559;
  static constexpr size_t bits = 8 * sizeof(T);
  static constexpr bool normalizes = is_iec559 && (bits == 32 || bits == 64);

  static inline T get_flushed(T value) {
    if constexpr(normalizes) {
      if constexpr(bits == 32) {
        union {
          T f;
          int32_t i;
        } v;

        v.f = value;

        return v.i & 0x7f800000 ? value : 0.0f;
      }
      else if constexpr (bits == 64) {
        union {
          T f;
          int64_t i;
        } v;

        v.f = value;

        return v.i & 0x7ff0000000000000L ? value : 0;
      }
    }
    return value;
  }

  static inline void flush(T &value) {
    if constexpr(normalizes) {
      value = get_flushed(value);
    }
  }


  /**
   * Returns value, where value if set to zero if it is denormal and the
   * floating point type is iec559 compliant.
   */
  static inline T flush_and_get(T &value) {
    if constexpr(normalizes) {
      return (value = get_flushed(value));
    }
    return value;
  }

};

class FTZ {

public:
  /**
   * Returns whether normalization for the floating point type is supported.
   * Support is purely algorithmic and says nothing about performance impact of
   * denormal numbers on the particular processor architecture.
   */
  template <typename T> static constexpr bool supported() noexcept {
    return FTZFor<T>::normalizes;
  }

  /**
   * Flushes value to zero if it is denormal and the floating point type is
   * iec559 compliant and does nothing otherwise.
   */
  template <typename T> static inline void flush(T &value) noexcept {
    FTZFor<T>::flush(value);
  }

  /**
   * Returns value or zero if it is denormal and the floating point type is
   * iec559 compliant.
   */
  template <typename T>
  static inline T get_flushed(T value) noexcept {
    return FTZFor<T>::get_flushed(value);
  }

  /**
   * Returns value, where value if set to zero if it is denormal and the
   * floating point type is iec559 compliant.
   */
  template <typename T> static inline T flush_and_get(T &value) {
    return FTZFor<T>::flush_and_get(value);
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_DENORMAL_H
