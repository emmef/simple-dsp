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

enum class WrappingType { BIT_MASK, MODULO };

namespace base {

template <WrappingType wrappingType, size_t element_size = 1,
          typename size_type = size_t,
          int max_size_bits = 0>
struct WrappedBase;

template <size_t element_size, typename size_type, int max_size_bits>
struct WrappedBase<WrappingType::BIT_MASK, element_size, size_type,
                   max_size_bits> {

  using Size = Size<element_size, size_type, max_size_bits>;

  static constexpr size_type max_element_count = Size::max_bit_mask + 1;

  sdsp_nodiscard static bool
  is_valid_element_count(size_type minimum_element_count) {
    return minimum_element_count > 0 &&
           minimum_element_count <= max_element_count;
  }

  sdsp_nodiscard static size_type
  get_allocation_size_for(size_type minimum_element_count) noexcept {
    return is_valid_element_count(minimum_element_count)
               ? Bits<size_type>::bit_mask_including(
                     std::max(2, minimum_element_count) - 1) +
                     size_type(1)
               : 0;
  }

  WrappedBase(size_type minimum_element_count)
      : mask_(valid_mask(minimum_element_count)) {}

  size_type size() const noexcept { return mask_ + 1; }

  sdsp_nodiscard inline size_type wrapped(size_type to_wrap) const noexcept {
    return to_wrap & mask_;
  }

  sdsp_nodiscard inline size_type unsafe_inc(size_type index) const noexcept {
    return wrapped(index + 1);
  }

  sdsp_nodiscard inline size_type unsafe_dec(size_type index) const noexcept {
    return wrapped(index - 1);
  }

  sdsp_nodiscard size_type unsafe_add(size_type index,
                                      size_type delta) const noexcept {
    return (index + delta) & mask_;
  }

  sdsp_nodiscard size_type unsafe_sub(size_type index,
                                      size_type delta) const noexcept {
    return (index + mask_ + 1 - delta & mask_) & mask_;
  }

  sdsp_nodiscard inline size_type inc(size_type index) const noexcept {
    return unsafe_inc(index);
  }

  sdsp_nodiscard inline size_type dec(size_type index) const noexcept {
    return unsafe_dec(index);
  }

  sdsp_nodiscard size_type add(size_type index,
                               size_type delta) const noexcept {
    return unsafe_add(index, delta);
  }

  sdsp_nodiscard size_type sub(size_type index,
                               size_type delta) const noexcept {
    return unsafe_sub(index, delta);
  }

  void set_element_count(size_type minimum_element_count) {
    mask_ = valid_mask(minimum_element_count);
  }

private:
  static size_type valid_mask(size_t elements) {
    if (is_valid_element_count(elements)) {
      return Bits<size_type>::bit_mask_including(elements - 1);
    }
    throw std::invalid_argument(
        "WrappedIndex(BIT_MASK): invalid number of elements.");
  }
  size_type mask_;
};

template <size_t element_size, typename size_type, int max_size_bits>
struct WrappedBase<WrappingType::MODULO, element_size, size_type,
                   max_size_bits> {

  using Size = Size<element_size, size_type, max_size_bits>;

  static constexpr size_type max_element_count = Size::max_index / 2;

  sdsp_nodiscard static bool
  is_valid_element_count(size_type minimum_element_count) {
    return minimum_element_count > 0 &&
           minimum_element_count <= max_element_count;
  }

  sdsp_nodiscard static size_type
  get_allocation_size_for(size_type minimum_element_count) noexcept {
    return is_valid_element_count(minimum_element_count) ? minimum_element_count
                                                         : 0;
  }

  WrappedBase(size_type minimum_element_count)
      : size_(valid_element_count(minimum_element_count)) {}

  size_type size() const noexcept { return size_; }

  sdsp_nodiscard inline size_type wrapped(size_type to_wrap) const noexcept {
    return to_wrap % size_;
  }

  sdsp_nodiscard size_type unsafe_inc(size_type index) const noexcept {
    return (index + 1) % size_;
  }

  sdsp_nodiscard size_type unsafe_dec(size_type index) const noexcept {
    return (index - 1) % size_;
  }

  sdsp_nodiscard size_type unsafe_add(size_type index,
                                      size_type delta) const noexcept {
    return wrapped(index + delta);
  }

  sdsp_nodiscard size_type unsafe_sub(size_type index,
                                      size_type delta) const noexcept {
    return wrapped(index + size_ - delta);
  }

  sdsp_nodiscard size_type inc(size_type index) const noexcept {
    return unsafe_inc(wrapped(index));
  }

  sdsp_nodiscard size_type dec(size_type index) const noexcept {
    return unsafe_dec(wrapped(index));
  }

  sdsp_nodiscard size_type add(size_type index,
                               size_type delta) const noexcept {
    return unsafe_add(wrapped(index), wrapped(delta));
  }

  sdsp_nodiscard size_type sub(size_type index,
                               size_type delta) const noexcept {
    return unsafe_sub(wrapped(index), wrapped(delta));
  }

  void set_element_count(size_type minimum_element_count) {
    size_ = valid_element_count(minimum_element_count);
  }

private:
  size_type size_;

  sdsp_nodiscard static size_t valid_mask(size_type minimum_element_count) {
    if (is_valid_element_count(minimum_element_count)) {
      return minimum_element_count;
    }
    throw std::invalid_argument(
        "WrappedIndex(MODULO): number of elements must be non-zero and not greater than WrappedIndex::max_element_count.");
  }
};

} // namespace base

/**
 * Specifies a circular indexing model to address elements of size element_size
 * with indices of size_type and an optional llimited number of addressing bits
 * max_size_bits.
 *
 * The model can be applied in circular buffers and the like.
 * @see simpledsp::Size<element_size, size_type, max_size_bits>.
 */
template <WrappingType wrappingType, size_t element_size = 1,
          typename size_type = size_t,
          int max_size_bits = sizeof(size_type) * 8>
struct WrappedIndex : public base::WrappedBase<wrappingType, element_size,
                                               size_type, max_size_bits> {

  using Super =
      base::WrappedBase<wrappingType, element_size, size_type, max_size_bits>;
  using Size = Size<element_size, size_type, max_size_bits>;

  static constexpr size_type max_element_count = Super::max_element_count;

  /**
   * @return true if the minimum element count can be represented by this
   * wrapped index model type and false otherwise.
   */
  sdsp_nodiscard static bool
  is_valid_element_count(size_type minimum_element_count) noexcept {
    return Super::is_valid_element_count(minimum_element_count);
  }

  /**
   * @returns the amount of memory to allocate for a model that supports the
   * number of elements or zero if that is not possible.
   */
  sdsp_nodiscard static size_type
  get_allocation_size_for(size_type minimum_element_count) noexcept {
    return Super::get_allocation_size_for(minimum_element_count);
  }

  /**
   * Creates a circular indexing model that can address at least element_count
   * elements; or throws std::invalid_argument if the element_count is less than
   * two or too big to be represented because of size constraints.
   * @param element_count the minimum number of elements that should be
   * addressible.
   * @return a proper circular mask or zero if that is not possible.
   */
  explicit WrappedIndex(size_t element_count) : Super(element_count) {}

  /**
   * @return the maximum size of an array that can be addressed with this index
   * model.
   */
  sdsp_nodiscard size_type size() const noexcept { return Super::size(); }

  /**
   * @return the wrapped value of index.
   */
  sdsp_nodiscard inline size_type wrapped(size_type to_wrap) const noexcept {
    return Super::wrapped(to_wrap);
  }

  /**
   * @return the incremented, then wrapped value of index.
   */
  sdsp_nodiscard inline size_type inc(size_type index) const noexcept {
    return Super::inc(index);
  }

  /**
   * @return the decremented, then wrapped value of index.
   */
  sdsp_nodiscard inline size_type dec(size_type index) const noexcept {
    return Super::dec(index);
  }

  /**
   * @return index plus delta wrapped..
   */
  sdsp_nodiscard inline size_type add(size_type index,
                                      size_type delta) const noexcept {
    return Super::add(index, delta);
  }

  /**
   * @return index minus delta wrapped
   */
  sdsp_nodiscard inline size_type sub(size_type index,
                                      size_type delta) const noexcept {
    return Super::sub(index, delta);
  }

  /**
   * the decremented, then wrapped value of index.
   * This variant assumes that all arguments are size() or smaller. If that
   * assumption cannot be made, use the "safe" variant without the unsafe_
   * prefix instead. Rest assured that performance of the "safe" variant is as
   * good as the unsafe variant for implementations that do not need the
   * argument assumption for correctness.
   * @return the incremented, then wrapped value of index.
   */
  sdsp_nodiscard inline size_type unsafe_inc(size_type index) const noexcept {
    return Super::unsafe_inc(index);
  }

  /**
   * Returns the incremented, then wrapped value of index.
   * This variant assumes that all arguments are size() or smaller. If that
   * assumption cannot be made, use the "safe" variant without the unsafe_
   * prefix instead. Rest assured that performance of the "safe" variant is as
   * good as the unsafe variant for implementations that do not need the
   * argument assumption for correctness.
   * @return the decremented, then wrapped value of index.
   */
  sdsp_nodiscard inline size_type unsafe_dec(size_type index) const noexcept {
    return Super::unsafe_dec(index);
  }

  /**
   * Returns  index plus delta wrapped.
   * This variant assumes that all arguments are size() or smaller. If that
   * assumption cannot be made, use the "safe" variant without the unsafe_
   * prefix instead. Rest assured that performance of the "safe" variant is as
   * good as the unsafe variant for implementations that do not need the
   * argument assumption for correctness.
   * @return index plus delta wrapped..
   */
  sdsp_nodiscard inline size_type unsafe_add(size_type index,
                                             size_type delta) const noexcept {
    return Super::unsafe_add(index, delta);
  }

  /**
   * Returns index minus delta wrapped.
   * This variant assumes that all arguments are size() or smaller. If that
   * assumption cannot be made, use the "safe" variant without the unsafe_
   * prefix instead. Rest assured that performance of the "safe" variant is as
   * good as the unsafe variant for implementations that do not need the
   * argument assumption for correctness.
   * @return index minus delta wrapped
   */
  sdsp_nodiscard inline size_type unsafe_sub(size_type index,
                                             size_type delta) const noexcept {
    return Super::unsafe_sub(index, delta);
  }

  /**
   * Sets a new element count if that is valid and throws std::invalif_argument
   * otherwise. The actual possible number of elements can be bigger and is
   * returned by size().
   */
  void set_element_count(size_type element_count) {
    Super::set_element_count(element_count);
  }
};

template <WrappingType wrappingType, typename Element,
          typename size_type = size_t,
          int max_size_bits = sizeof(size_type) * 8>
using WrappedIndexFor =
    WrappedIndex<wrappingType, sizeof(Element), size_type, max_size_bits>;

/**
 * A bit-mask-based wrapped indexing model for the specified element type.
 */
template <typename Element, typename size_type = size_t,
          int max_size_bits = sizeof(size_type) * 8>
using MaskedIndexFor = WrappedIndex<WrappingType::BIT_MASK, sizeof(Element),
                                    size_type, max_size_bits>;

/**
 * A modulo-based wrapped indexing model for the specified element type.
 */
template <typename Element, typename size_type = size_t,
          int max_size_bits = sizeof(size_type) * 8>
using ModuloIndexFor = WrappedIndex<WrappingType::MODULO, sizeof(Element),
                                    size_type, max_size_bits>;

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_CIRCULAR_H
