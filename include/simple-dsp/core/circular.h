#ifndef SIMPLE_DSP_CORE_CIRCULAR_H
#define SIMPLE_DSP_CORE_CIRCULAR_H
/*
 * simple-dsp/core/circular.h
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
#include <simple-dsp/core/attributes.h>
#include <simple-dsp/core/size.h>
#include <type_traits>

namespace simpledsp {

template <typename SIZE = size_t> struct CircularArithmic {
  using size_type = SIZE;
  using Type = SizeType<size_type>;

  static constexpr size_type maximumMask = SizeType<size_type >::max_bit_mask;

  sdsp_nodiscard static constexpr size_type
  proper_circular_size(size_type requestedSize) {

    return requestedSize > static_cast<size_type>(2)
           ? Power2::same_or_bigger(requestedSize)
           : static_cast<size_type>(2);
  }

  sdsp_nodiscard static constexpr size_type
  proper_circular_mask(size_type requestedSize) {

    return Bits<size_type>::surrounding_mask(requestedSize) & maximumMask;
  }

  sdsp_nodiscard static constexpr size_type next(size_type circularPointer,
                                              size_type uncheckedMask) {

    return (circularPointer + 1) & uncheckedMask;
  }

  sdsp_nodiscard static constexpr size_type previous(size_type circularPointer,
                                                  size_type uncheckedMask) {

    return (circularPointer - 1) & uncheckedMask;
  }

  sdsp_nodiscard static constexpr size_type size(size_type uncheckedMask) {

    return uncheckedMask + 1;
  }

  sdsp_nodiscard static constexpr size_type
  add(size_type circularPointer, size_type delta, size_type uncheckedMask) {

    return (circularPointer + delta) & uncheckedMask;
  }

  sdsp_nodiscard static constexpr size_type
  subtract(size_type circularPointer, size_type delta, size_type uncheckedMask) {

    return (circularPointer + uncheckedMask + 1 - delta & uncheckedMask) &
           uncheckedMask;
  }

  sdsp_nodiscard static constexpr size_type
  subtract_unsafe(size_type circularPointer, size_type delta, size_type uncheckedMask) {

    return (delta & circularPointer + uncheckedMask + 1 - delta &
            uncheckedMask) &
           uncheckedMask;
  }

  static constexpr void set_next(size_type &circularPointer, size_type uncheckedMask) {

    ++circularPointer &= uncheckedMask;
  }

  static constexpr void set_previous(size_type &circularPointer,
                                    size_type uncheckedMask) {

    --circularPointer &= uncheckedMask;
  }
};

template <typename size_type = size_t> class CircularMetric {
  size_type mask;
  using Arithmic = CircularArithmic<size_type>;

public:
  explicit CircularMetric(Size<size_type> requestedSize)
      : mask(Arithmic::proper_circular_mask(requestedSize)) {}

  sdsp_nodiscard size_type getSize() const { return mask + 1; }

  sdsp_nodiscard size_type getMask() const { return mask; }

  sdsp_nodiscard size_type wrap(size_type toWrap) const { return toWrap & mask; }

  sdsp_nodiscard size_type next(size_type pointer) const {
    return Arithmic::next(pointer, mask);
  }

  sdsp_nodiscard size_type previous(size_type pointer) const {
    return Arithmic::previous(pointer, mask);
  }

  void setNext(size_type &pointer) const { Arithmic::set_next(pointer, mask); }

  void setPrevious(size_type &pointer) const {
    Arithmic::set_previous(pointer, mask);
  }

  sdsp_nodiscard size_type add(size_type pointer, size_type delta) const {
    return Arithmic::add(pointer, delta, mask);
  }

  sdsp_nodiscard size_type subtract(size_type pointer, size_type delta) const {
    return Arithmic::subtract(pointer, delta, mask);
  }

  sdsp_nodiscard bool setSize(size_t requestedSize) {
    size_type newMask = Arithmic::proper_circular_mask(requestedSize);
    if (newMask + 1 >= requestedSize) {
      mask = newMask;
      return true;
    }
    return false;
  }
};

//using CircularArithmic = base::CircularArithmic<size_t>;
//using CircularMetric = base::CircularMetric<size_t>;
//
} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_CIRCULAR_H
