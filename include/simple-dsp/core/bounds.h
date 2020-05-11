#ifndef SIMPLE_DSP_CORE_BOUNDS_H
#define SIMPLE_DSP_CORE_BOUNDS_H

/*
 * simple-dsp/core/bounds.h
 *
 * Added by michel on 2019-09-12
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

#include <algorithm>
#include <cstddef>
#include <simple-dsp/core/attributes.h>
#include <type_traits>

namespace simpledsp {

template <typename T>
sdsp_nodiscard static constexpr bool is_within(const T value, const T minimum,
                                               const T maximum) {
  return value == ::std::clamp(value, minimum, maximum);
}

template <typename T>
static constexpr bool is_within_excl(const T value, const T minimum,
                                     const T maximum) {
  return value > minimum && value < maximum;
}

struct Unsigned {

  template <typename T> static constexpr T uint_check(T value) noexcept {
    static_assert(
        std::is_integral<T>::value && !std::is_signed<T>::value &&
            sizeof(T) <= sizeof(size_t),
        "Expected an unsigned, integral type not larger than size_t.");
    return value;
  }

  template <typename T>
  static constexpr bool is_not_greater(T value, T max) noexcept {
    return uint_check(value) <= max;
  }

  template <typename destination_type, typename source_type>
  static constexpr bool
  is_not_greater(source_type source_value,
                 destination_type destination_maximum) noexcept {
    return size_t(uint_check(source_value)) <=
           size_t(uint_check(destination_maximum));
  }

  template <typename T>
  static constexpr bool is_nonzero_not_greater(T value, T max) noexcept {
    return uint_check(value) && value <= max;
  }

  template <typename destination_type, typename source_type>
  static constexpr bool
  is_nonzero_not_greater(source_type source_value,
                         destination_type destination_maximum) noexcept {
    return source_value && is_not_greater(source_value, destination_maximum);
  }

  template <typename T>
  static constexpr bool is_sum_nonzero_not_greater(T v1, T v2, T max) noexcept {
    return uint_check(v1) > 0 ? v1 <= max && max - v1 >= v2
                              : v2 > 0 && v2 <= max;
  }

  template <typename T>
  static constexpr bool
  is_product_nonzero_not_greater(T v1, T v2, size_t size_max) noexcept {
    return uint_check(v1) > 0 && v2 > 0 && size_max / v1 >= v2;
  }

  template <typename T>
  static constexpr bool is_sum_not_greater(T v1, T v2, T max) noexcept {
    return uint_check(v1) <= max && max - v1 >= v2;
  }

  template <typename T>
  static constexpr bool is_product_not_greater(T v1, T v2,
                                               size_t max) noexcept {
    return uint_check(v1) == 0 || (v1 <= max && max / v1 >= v2);
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_BOUNDS_H
