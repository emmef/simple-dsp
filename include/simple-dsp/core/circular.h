#ifndef SIMPLE_DSP_CIRCULAR_H
#define SIMPLE_DSP_CIRCULAR_H
/*
 * simple-dsp/algorithm/circular.h
 *
 * Added by michel on 2019-08-19
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

#include <limits>
#include <simple-dsp/core/addressing.h>
#include <simple-dsp/core/algorithm.h>
#include <simple-dsp/core/attributes.h>
#include <type_traits>

namespace simpledsp {

namespace base {
template <typename SIZE_T> struct CircularArithmic {
  static_assert(std::is_integral<SIZE_T>::value &&
                !std::is_signed<SIZE_T>::value,
                "SIZE_T must be an integral, unsigned type");

  static constexpr SIZE_T maximumMask = addr::Elements<char,SIZE_T,0>::Index::max >> 1;

  sdsp_nodiscard static constexpr SIZE_T
  proper_circular_size(SIZE_T requestedSize) {

    return requestedSize > static_cast<SIZE_T>(2)
           ? Power2::same_or_bigger(requestedSize)
           : static_cast<SIZE_T>(2);
  }

  sdsp_nodiscard static constexpr SIZE_T
  proper_circular_mask(SIZE_T requestedSize) {

    return Power2::surrounding_mask(requestedSize) & maximumMask;
  }

  sdsp_nodiscard static constexpr SIZE_T next(SIZE_T circularPointer,
                                              SIZE_T uncheckedMask) {

    return (circularPointer + 1) & uncheckedMask;
  }

  sdsp_nodiscard static constexpr SIZE_T previous(SIZE_T circularPointer,
                                                  SIZE_T uncheckedMask) {

    return (circularPointer - 1) & uncheckedMask;
  }

  sdsp_nodiscard static constexpr SIZE_T size(SIZE_T uncheckedMask) {

    return uncheckedMask + 1;
  }

  sdsp_nodiscard static constexpr SIZE_T
  add(SIZE_T circularPointer, SIZE_T delta, SIZE_T uncheckedMask) {

    return (circularPointer + delta) & uncheckedMask;
  }

  sdsp_nodiscard static constexpr SIZE_T
  subtract(SIZE_T circularPointer, SIZE_T delta, SIZE_T uncheckedMask) {

    return (circularPointer + uncheckedMask + 1 - delta & uncheckedMask) &
           uncheckedMask;
  }

  sdsp_nodiscard static constexpr SIZE_T
  subtract_unsafe(SIZE_T circularPointer, SIZE_T delta, SIZE_T uncheckedMask) {

    return (delta & circularPointer + uncheckedMask + 1 - delta &
            uncheckedMask) &
           uncheckedMask;
  }

  static constexpr void set_next(SIZE_T &circularPointer, SIZE_T uncheckedMask) {

    ++circularPointer &= uncheckedMask;
  }

  static constexpr void set_previous(SIZE_T &circularPointer,
                                    SIZE_T uncheckedMask) {

    --circularPointer &= uncheckedMask;
  }
};

template <typename SIZE_T> class CircularMetric {
  SIZE_T mask;
  using Arithmic = CircularArithmic<SIZE_T>;

public:
  explicit CircularMetric(SIZE_T requestedSize)
      : mask(Arithmic::proper_circular_mask(requestedSize)) {}

  sdsp_nodiscard SIZE_T getSize() const { return mask + 1; }

  sdsp_nodiscard SIZE_T getMask() const { return mask; }

  sdsp_nodiscard SIZE_T wrap(SIZE_T toWrap) const { return toWrap & mask; }

  sdsp_nodiscard SIZE_T next(SIZE_T pointer) const {
    return Arithmic::next(pointer, mask);
  }

  sdsp_nodiscard SIZE_T previous(SIZE_T pointer) const {
    return Arithmic::previous(pointer, mask);
  }

  void setNext(SIZE_T &pointer) const { Arithmic::set_next(pointer, mask); }

  void setPrevious(SIZE_T &pointer) const {
    Arithmic::set_previous(pointer, mask);
  }

  sdsp_nodiscard SIZE_T add(SIZE_T pointer, SIZE_T delta) const {
    return Arithmic::add(pointer, delta, mask);
  }

  sdsp_nodiscard SIZE_T subtract(SIZE_T pointer, SIZE_T delta) const {
    return Arithmic::subtract(pointer, delta, mask);
  }

  sdsp_nodiscard bool setSize(size_t requestedSize) {
    SIZE_T newMask = Arithmic::proper_circular_mask(requestedSize);
    if (newMask + 1 >= requestedSize) {
      mask = newMask;
      return true;
    }
    return false;
  }
};

} // namespace base

using CircularArithmic = base::CircularArithmic<size_t>;
using CircularMetric = base::CircularMetric<size_t>;

} // namespace simpledsp

#endif // SIMPLE_DSP_CIRCULAR_H
