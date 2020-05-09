#ifndef SIMPLE_DSP_CORE_BITS_H
#define SIMPLE_DSP_CORE_BITS_H
/*
 * simple-dsp/core/bits.h
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

namespace simpledsp {

template <typename size_type = size_t> class Bits {
  static_assert(std::is_integral<size_type>::value &&
                    !std::is_signed<size_type>::value,
                "PowerOfTwo:: Type must be an integral, unsigned type");

  template <int N> static constexpr size_type fillN(const size_type n) {
    return N < 2 ? n : fillN<N / 2>(n) | (fillN<N / 2>(n) >> (N / 2));
  };

public:
  /**
   * Fill all bits that are less siginificant than the most significant bit.
   * @param value The value to fill bits
   * @return value with all bits set that are less siginificant than the most
   * significant bit.
   */
  static constexpr size_type fill(const size_type value) {
    return fillN<8 * sizeof(size_type)>(value);
  };

  /**
   * @param value The value to test.
   * @return The most significant set bit number.
   * Bit numbers range from zero to sizeof(size_type) - 1.
   */
  static constexpr int most_significant(const size_type value) {
    int bit = sizeof(size_type) * 8 - 1;
    while (bit >= 0 && (value & (size_type(1) << bit)) == 0) {
      bit--;
    }
    return bit;
  }

  /**
   * @param value The value to test.
   * @return The  most significant set bit number if only a single bit is set or
   * - 1 minus the next most singnificant bit set. Bit numbers range from zero
   * to sizeof(size_type) - 1.
   */
  static constexpr int most_significant_single(const size_type value) {
    int bit = sizeof(size_type) * 8 - 1;
    while (bit >= 0 && (value & (size_type(1) << bit)) == 0) {
      bit--;
    }
    if (bit < 1) {
      return bit;
    }
    int lowerbit = bit - 1;
    while (lowerbit >= 0) {
      if (value & (size_type(1) << lowerbit)) {
        return -lowerbit - 1;
      }
      lowerbit--;
    }
    return bit;
  }

  /**
   * Returns an offset mask to use for offsets inside a space of size
   * same_or_bigger(value)..
   */
  static constexpr size_type surrounding_mask(const size_type value) {
    return value <= 2 ? 1 : fill(value - 1);
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_BITS_H
