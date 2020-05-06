#ifndef SIMPLE_DSP_ALGORITHM_H
#define SIMPLE_DSP_ALGORITHM_H
/*
 * simple-dsp/algorithm.h
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
#include <type_traits>
#include <simple-dsp/core/attributes.h>

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

namespace internal {
namespace {

template <typename SIZE_T, size_t SIZE_OF_SIZE_T> struct BaseFillBitsToRight {};

template <class SIZE_T> struct BaseFillBitsToRight<SIZE_T, 1> {
  static inline SIZE_T fill(const SIZE_T x) {
    SIZE_T n = x;
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    return n;
  }
};

template <typename SIZE_T> struct BaseFillBitsToRight<SIZE_T, 2> {
  static inline SIZE_T fill(const SIZE_T x) {
    SIZE_T n = x;
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    n = n | (n >> 8);
    return n;
  }
};

template <typename SIZE_T> struct BaseFillBitsToRight<SIZE_T, 4> {
  static inline SIZE_T fill(const SIZE_T x) {
    SIZE_T n = x;
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    n = n | (n >> 8);
    n = n | (n >> 16);
    return n;
  }
};

template <typename SIZE_T> struct BaseFillBitsToRight<SIZE_T, 8> {
  static inline SIZE_T fill(const SIZE_T x) {
    SIZE_T n = x;
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    n = n | (n >> 8);
    n = n | (n >> 16);
    n = n | (n >> 32);
    return n;
  }
};

template <typename SIZE_T> struct BaseFillBitsToRight<SIZE_T, 16> {
  static inline SIZE_T fill(const SIZE_T x) {
    SIZE_T n = x;
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    n = n | (n >> 8);
    n = n | (n >> 16);
    n = n | (n >> 32);
    n = n | (n >> 64);
    return n;
  }
};

template <typename SIZE_T> class BaseFillBitsToRight<SIZE_T, 0> {
  template <size_t N> static constexpr SIZE_T fillN(const SIZE_T n) {
    return N < 2 ? n : fillN<N / 2>(n) | (fillN<N / 2>(n) >> (N / 2));
  };

public:
  static constexpr SIZE_T fill(const SIZE_T n) {
    return fillN<8 * sizeof(SIZE_T)>(n);
  };
};

} // anonymous namespace
}

template <bool constExpr, typename SIZE_T> class BasePowerOfTwo {
  static_assert(std::is_integral<SIZE_T>::value &&
                !std::is_signed<SIZE_T>::value,
                "Type must be an integral, unsigned type");

  using BitFiller =
  internal::BaseFillBitsToRight<size_t, constExpr ? 0 : sizeof(size_t)>;

  static constexpr SIZE_T fill(SIZE_T value) { return BitFiller::fill(value); }

  static constexpr SIZE_T getAligned(SIZE_T value, SIZE_T alignment) {
    SIZE_T filled = fill(alignment >> 1);
    return (value + filled) & ~filled;
  }

public:
  static constexpr bool isMinusOne(const SIZE_T value) {
    return value != 0 && fill(value) == value;
  }

  static constexpr bool is(const SIZE_T value) {
    return value >= 2 && isMinusOne(value - 1);
  }

  /**
   * Returns value if it is a power of two or else the next power of two that is
   * greater.
   */
  static constexpr SIZE_T sameOrBigger(const SIZE_T value) {
    return value <= 2 ? 2 : fill(value - 1) + 1;
  }

  /**
   * Returns an offset mask to use for offsets inside a space of size
   * sameOrBigger(value)..
   */
  static constexpr SIZE_T surroundingMask(const SIZE_T value) {
    return fill(value - 1);
  }

  /**
   * Returns the value if it is already a multiple of power_of_two or the
   * next multiple of power_of_two otherwise. If power_of_two is not a power
   * of two, the alignment is done with the next higher power of two.
   *
   * @param value Value to be aligned
   * @param power_of_two The power of two to align to, or the next bigger power
   * of two.
   * @return the aligned value
   */
  static constexpr SIZE_T alignedWith(const SIZE_T value,
                                      const SIZE_T power_of_two) {
    return getAligned(value, power_of_two);
  }
};

using Power2 = BasePowerOfTwo<false, size_t>;
using Power2Const = BasePowerOfTwo<true, size_t>;
} // namespace simpledsp


#endif // SIMPLE_DSP_ALGORITHM_H
