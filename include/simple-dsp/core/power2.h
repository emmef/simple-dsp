#ifndef SIMPLE_DSP_POWER2_H
#define SIMPLE_DSP_POWER2_H
/*
 * simple-dsp/core/power2.h
 *
 * Added by michel on 2020-05-09
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
#include <simple-dsp/core/bits.h>

namespace simpledsp {

template <typename size_type = size_t> struct PowerTwo {

  /**
   * Returns whether value is a power of two.
   * @param value The value.
   * @return true if value if a power of two, false oherwise.
   */
  static constexpr bool is(const size_type value) {
    return value >= 2 && is_minus_one(value - 1);
  }

  /**
   * Returns whether value is a power of two minus one.
   * @param value The value.
   * @return true if value if a power of two minus one, false oherwise.
   */
  static constexpr bool is_minus_one(const size_type value) {
    return value != 0 && Bits<size_type>::fill(value) == value;
  }

  /**
   * Returns value if it is a power of two or else the next greater power of
   * two.
   * @param The value
   * @return value if it is a power of two or else the next greater power of
   * two.
   */
  static constexpr size_type same_or_bigger(const size_type value) {
    return value <= 2 ? 2 : Bits<size_type>::fill(value - 1) + 1;
  }
};

struct Power2 {
  template <typename size_type>
  static constexpr size_type fill(const size_type n) {
    return PowerTwo<size_type>::fill(n);
  };

  template <typename size_type>
  static constexpr bool is_minus_one(const size_type value) {
    return PowerTwo<size_type>::is_minus_one(value);
  }

  template <typename size_type>
  static constexpr bool is(const size_type value) {
    return PowerTwo<size_type>::is(value);
  }

  template <typename size_type>
  static constexpr size_type same_or_bigger(const size_type value) {
    return PowerTwo<size_type>::same_or_bigger(value);
  }
};


} // namespace simpledsp

#endif // SIMPLE_DSP_POWER2_H
