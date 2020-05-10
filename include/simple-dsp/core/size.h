#ifndef SIMPLE_DSP_CORE_SIZE_H
#define SIMPLE_DSP_CORE_SIZE_H
/*
 * simple-dsp/core/size.h
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

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <simple-dsp/core/power2.h>

namespace simpledsp {

/**
 * Processes almost never have access to the full address and size ranges
 * that "fits" in size_t. Most hardware and operating systems reserve bits for
 * special purposes, making them inaccessible for the lineair virtual address
 * range.The number of these bits is also such, that you at least have 7 bits of
 * addressing spacel left.
 *
 * The number can be overridden at compile time with a predefined constant
 * SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS (naturally clamped between the
 * minimum and maximum.
 *
 * For systems with more than 32 bits, the value is estimated, based on 64 bit
 * intel systems that reserve 17 bits for privilege levels and paging that thus
 * cannot be used for the lineair virtual addressing space of the process.
 * Suggestions for a more accurate guess are most certainly welcome!
 */
static constexpr int stolen_address_bits =
#if SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS > 0
    std::clamp(SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS,
               sizeof(size_t) < 4 ? 0 : 1, sizeof(size_t) * 8 - 7);
#else
    sizeof(size_t) <= 4 ? 1 : 1 + 8 * (sizeof(size_t) / 4);
#endif

/**
 * The platform-dependent effective number of bits than can be used for
 * size-values.
 */
static constexpr int effective_size_t_bits =
    sizeof(size_t) * 8 - stolen_address_bits;

/**
 * Return whether the size is valid: non-zero and not greater than size_max.
 * @return true if size is valid, false otherwise.
 */
template <typename size_type>
static constexpr bool size_is_valid(size_type size, size_t size_max) noexcept {
  return size > 0 && size <= size_max;
}

/**
 * Return whether index is valid: not greater than index_max.
 * @return true if index is valid, false otherwise.
 */
template <typename size_type>
static constexpr bool size_is_valid_index(size_type index,
                                          size_t index_max) noexcept {
  return index <= index_max;
}

/**
 * Returns whether size, specific in the source size type, can be assigned as
 * valid size in the destination size type that has an applied limit of
 * size_max.
 *
 * @tparam destination_size_type
 * @tparam source_size_type
 * @param size The size in the source size type.
 * @param size_max The specified maximum for the destination size in the
 * destination size type.
 * @return true if the size can be assigned, false otherwise.
 */
template <typename destination_size_type, typename source_size_type>
static constexpr bool size_can_assign(source_size_type size,
                                      destination_size_type size_max) noexcept {
  static_assert(
      sizeof(destination_size_type) <= sizeof(size_t),
      "size_can_assing: dst_size_type cannot be larger than std::size_t.");
  static_assert(
      sizeof(source_size_type) <= sizeof(size_t),
      "size_can_assign: src_size_type cannot be larger than std::size_t.");

  return size > 0 && size_t(size) <= size_t(size_max);
}

/**
 * Returns whether the sum of two values v1 and v2, represents a valid size that
 * is non-zero and not greater than size_max.
 * @return true if the sum represents a valid size, false otherwise.
 */
template <typename size_type>
static constexpr bool size_is_valid_sum(size_type v1, size_type v2,
                                        size_type size_max) noexcept {
  return v1 > 0 ? v1 <= size_max && size_max - v1 >= v2
                : v2 > 0 && v2 <= size_max;
}

/**
 * Returns whether the product of two values v1 and v2, represents a valid size
 * that is non-zero and not greater than size_max.
 * @return true if the product represents a valid size, false otherwise.
 */
template <typename size_type>
static constexpr bool size_is_valid_product(size_type v1, size_type v2,
                                            size_t size_max) noexcept {
  return v1 > 0 && v2 > 0 && size_max / v1 >= v2;
}

/**
 * Describes properties of sizes that are stored in the underlying SIZE data
 * type, that must be integral, unsigned and cannot be larger than std::size_t.
 * Sizes can be artificially limited to a reduced number of bits indicated by
 * size_bit_limit, with a minimum of 8.
 *
 * @tparam SIZE The integral, unsigned type that contains sizes.
 * @tparam size_bit_limit The artifically reduced number of bits to represent
 * sizes.
 */
template <typename SIZE = size_t, int size_bit_limit = sizeof(SIZE) * 8>
struct SizeType {
  static_assert(
      std::is_integral<SIZE>::value,
      "simpledsp::size::system::SizeType: size_type must be an integral type.");

  static_assert(
      !std::is_signed<SIZE>::value,
      "simpledsp::size::system::SizeType: size_type must be an unsigned type.");

  static_assert(sizeof(SIZE) <= sizeof(size_t),
                "simpledsp::size::system::SizeType: size_type cannot be larger "
                "than std::size_t.");

  static_assert(size_bit_limit > 7,
                "SizeType: explicit size_bit_limit must be at least 7.");

  static_assert(size_bit_limit <= sizeof(SIZE) * 8,
                "SizeType: explicit size_bit_limit cannot exceed number of "
                "bits in size_type.");

  using size_type = SIZE;

  /**
   * The number of bits in the underlying size_type.
   */
  static constexpr int type_bits = 8 * sizeof(size_type);

  /**
   * The maximum value that can be represented by the underlying size_type.
   */
  static constexpr int type_max = std::numeric_limits<size_type>::max();

  /**
   * The number of bits that is usable due to system limits or an explicit limit
   * set by size_bit_limit.
   */
  static constexpr int size_bits =
      std::min(size_bit_limit, effective_size_t_bits);

  /**
   * The maximum value of a size.
   */
  static constexpr size_type max = type_bits == size_bits
                                       ? std::numeric_limits<size_type>::max()
                                       : size_type(1) << size_bits;

  /**
   * The maximum index to address structures with a size. This is equal to max
   * if max is equal to std::numeric_limits<size_type>::max() and max minus one
   * otherwise. This value is always a power of two minus one.
   */
  static constexpr size_type max_index =
      type_bits == size_bits ? std::numeric_limits<size_type>::max()
                             : (size_type(1) << size_bits) - 1;

  /**
   * The maximum bit mask that can be used for "wrapped" addressing structures
   * like circular buffers.
   */
  static constexpr size_type max_bit_mask =
      Bits<size_type>::bit_mask_not_exceeding(max_index);

  /**
   * Returns the maximum number of elements in an array with elements of the
   * specified size.
   * @param element_size The size of each element.
   * @return the maximum number of elements, that can be zero.
   */
  static constexpr size_type max_count_for_element_size(size_t element_size) {
    return max / std::max(size_t(1), element_size);
  }

  /**
   * Returns the maximum index to address an array with elements of the
   * specified size.
   * @param element_size The size of each element.
   * @return the maximum index, that can be zero.
   */
  static constexpr size_type max_index_for_element_size(size_t element_size) {
    return element_size > 1 ? max_count_for_element_size(element_size) - 1
                            : max_index;
  }

  /**
   * Returns the maximum bit mask that can be used for "wrapped" addressing
   * structures, when backed by an array with elements of the specified size.
   */
  static constexpr size_type
  max_bit_mask_for_element_size(size_t element_size) {
    return Bits<size_type>::bit_mask_not_exceeding(
        max_index_for_element_size(element_size));
  }

  /**
   * Returns whether size is valid: non-zero and not greater than max.
   * @return true if size if valid, false otherwise.
   */
  static constexpr bool is_valid(size_type size) noexcept {
    return size_is_valid(size, max);
  }

  /**
   * Returns whether index is valid: not greater than max_index.
   * @return true if index if valid, false otherwise.
   */
  static constexpr bool is_valid_index(size_type index) noexcept {
    return size_is_valid_index(index, max_index);
  }

  /**
   * Returns whether size, specific in the source size type, represents a valid
   * size in this SizeType<size_type>.
   * @tparam src_size_type the size type of size
   * @param size The size value in the source size type
   * @return true if the size can be represented, false otherwise.
   * @see is_valid(size_type)
   */
  template <typename src_size_type>
  static constexpr bool can_assign(src_size_type size) noexcept {
    return size_can_assign(size, max);
  }

  /**
   * Returns whether the sum of two values v1 and v2, represents a valid size
   * that is non-zero and not greater than max.
   * @return true if the sum represents a valid size, false otherwise.
   */
  static constexpr bool is_valid_sum(size_type v1, size_type v2) noexcept {
    return size_is_valid_sum(v1, v2, max);
  }

  /**
   * Returns whether the product of two values v1 and v2, represents a valid
   * size that is non-zero and not greater than max.
   * @return true if the product represents a valid size, false otherwise.
   */
  static constexpr bool is_valid_product(size_type v1, size_type v2) noexcept {
    return size_is_valid_product(v1, v2, max);
  }
};

static constexpr size_t system_max_size_in_bytes = SizeType<size_t>::max;

static constexpr size_t system_max_byte_index = SizeType<size_t>::max_index;

/**
 * Describes properties of sizes that are stored in the underlying SIZE data
 * type an that are used to address arrays with elements of type T. , that must
 * be integral, unsigned and cannot be larger than std::size_t. Sizes can be
 * artificially limited to a reduced number of bits indicated by size_bit_limit,
 * with a minimum of 8.
 * @tparam T
 * @tparam SIZE
 * @tparam max_size_bits
 */
template <size_t element_size, typename SIZE = std::size_t,
          int max_size_bits = sizeof(SIZE) * 8>
struct Size {
  static_assert(element_size > 0, "Size: element_size cannot be zero.");
  static_assert(
      element_size <= SizeType<SIZE, max_size_bits>::max,
      "Size: element_size cannot exceed maximum size than can be represented.");
  using size_type = SIZE;
  using Type = SizeType<size_type, max_size_bits>;

  /**
   * The maximum number of elements.
   */
  static constexpr size_type max =
      Type::max_count_for_element_size(element_size);

  /**
   * The maximum element index.
   */
  static constexpr size_type max_index =
      Type::max_index_for_element_size(element_size);

  /**
   * The maximum bit mask for "wrapped" index models. This is normally the same
   * as max_index, except in cases where max_index is equal to the numeric limit
   * of size_t. In that case, the size of a wrapped index model (max_index + 1)
   * cannot be represented by size_type and the max_bit_mask is the next smaller
   * power of two minus one.
   * r
   */
  static constexpr size_type
      max_bit_mask = max_index == Type::type_max
                         ? Type::max >> 1
                         : Bits<size_type>::bit_mask_not_exceeding(max_index);

  /**
   * Returns whether element_count is valid: non-zero and no greater than max.
   * @return true if element_count is valud, false otherwise.
   */
  sdsp_nodiscard static constexpr bool
  is_valid(size_type element_count) noexcept {
    return size_is_valid(element_count, max);
  }

  /**
   * Returns whether element_index is valid: non-zero and no greater than max.
   * @return true if element_count is valud, false otherwise.
   */
  sdsp_nodiscard static constexpr bool
  is_valid_index(size_type element_index) noexcept {
    return size_is_valid_index(element_index, max_index);
  }

  /**
   * Returns whether the sum of v1 and v2 represents a valid element count:
   * non-zero and no greater than max.
   * @return true if the sum represents a valid element count.
   */
  sdsp_nodiscard static constexpr bool is_valid_sum(size_type v1,
                                                    size_type v2) noexcept {
    return size_is_valid_sum(v1, v2, max);
  }

  /**
   * Returns whether the product of v1 and v2 represents a valid element count:
   * non-zero and no greater than max.
   * @return true if the product represents a valid element count.
   */
  sdsp_nodiscard static constexpr bool is_valid_product(size_type v1,
                                                        size_type v2) noexcept {
    return size_is_valid_product(v1, v2, max);
  }

  /**
   * Returns whether size represents a valid element count.
   * @tparam source_size_type the size type of size
   * @param size The size value in the source size type
   * @return true if the size can be represented, false otherwise.
   * @see is_valid(size_type)
   */
  template <typename source_size_type>
  sdsp_nodiscard static constexpr bool
  can_assign(source_size_type size) noexcept {
    return size_can_assign(size, max);
  }

  /**
   * Returns whether size represents a valid element count. The size must have
   * the same element_size.
   * @tparam source_size_type The size type of size
   * @param size The source size, that must have the same element_size.
   * @return true if the size can be represented, false otherwise.
   * @see is_valid(size_type)
   */
  template <typename source_size_type, source_size_type tmax>
  sdsp_nodiscard static constexpr bool
  can_assign(const Size<element_size, source_size_type, tmax> &size) noexcept {
    return can_assign(size.value);
  }

  /**
   * Returns element_count if it is valid and throws std::invalid_argument
   * otherwise.
   * @return element_size
   * @see is_valid(size_type)
   */
  static constexpr size_type get_valid_size(size_type element_count) {
    if (is_valid(element_count)) {
      return element_count;
    }
    throw std::invalid_argument(
        "Size: size must be positive and not greater than Size::max.");
  }

  /**
   * Returns element_index if it is valid and throws std::invalid_argument
   * otherwise.
   * @return element_index
   * @see is_valid(size_type)
   */
  static constexpr size_type get_valid_index(size_type element_index) {
    if (is_valid_index(element_index)) {
      return element_index;
    }
    throw std::invalid_argument(
        "Size: index must not be greater than Size::max_index");
  }

  /**
   * Returns the sum of v1 and v2 is it is valid and throws
   * std::invalid_argument otherwise.
   * @return the sum of v1 and v2.
   * @see is_valid_sum(size_type, size_type)
   */
  static constexpr size_type get_valid_sum(size_type v1, size_type v2) {
    if (is_valid_sum(v1, v2)) {
      return v1 + v2;
    }
    throw std::invalid_argument(
        "Size: sum must be positive and not greater than Size::max.");
  }

  /**
   * Returns the product of v1 and v2 is it is valid and throws
   * std::invalid_argument otherwise.
   * @return the product of v1 and v2.
   * @see is_valid_product(size_type, size_type)
   */
  static constexpr size_type get_valid_product(size_type v1, size_type v2) {
    if (is_valid_product(v1, v2)) {
      return v1 * v2;
    }
    throw std::invalid_argument(
        "Size: product must be positive and not greater than Size::max.");
  }

  /**
   * Returns the size if it represents a valid element count and throws
   * std::invalid_argument otherwise.
   * @return the value of size cast to size_type.
   */
  template <typename src_size_type>
  static constexpr size_type get_assignable_size(src_size_type size) {
    if (can_assign(size)) {
      return size;
    }
    throw std::invalid_argument(
        "Size: value is zero or too large to be assingned.");
  }

  /**
   * Returns the size if it represents a valid element count and throws
   * std::invalid_argument otherwise.
   * @return the value of size cast to size_type.
   */
  template <typename ST, ST tmax>
  static constexpr size_type
  get_assignable_size(const Size<element_size, ST, tmax> &size) {
    if (can_assign(size)) {
      return size;
    }
    throw std::invalid_argument(
        "Size: value is zero or too large to be assingned.");
  }

  /**
   * Construct a Size based on a valid element count or throws
   * std::invalid_argument otherwise.
   */
  explicit Size(const size_t element_count)
      : value(get_valid_size(element_count)) {}

  /**
   * @return value
   */
  sdsp_nodiscard operator size_type() const noexcept { return value; }

  /**
   * Explicitly returns value.
   * @return value
   */
  sdsp_nodiscard size_t operator()() const noexcept { return value; }

  /**
   * Returns the maximum element count from an instance.
   * @return the maximum element count, max.
   */
  sdsp_nodiscard size_t maximum() const noexcept { return max; }

  /**
   * Returns the maximum element index from an instance.
   * @return the maximum element index, max_index.
   */
  sdsp_nodiscard size_t maximum_index() const noexcept { return max_index; }

  /**
   * Returns the maximum bit mask from an instance.
   * @return the maximum bit mask, max_bit_mask.
   */
  sdsp_nodiscard size_t maximum_bit_mask() const noexcept {
    return max_bit_mask;
  }

  /**
   * Adds element count if that results in a valid total element count and
   * throws std::invalid_argument otherwise.
   */
  Size &operator+=(size_type element_count) {
    value = get_valid_sum(value, element_count);
    return *this;
  }

  /**
   * Adds element count if that results in a valid total element count and
   * throws std::invalid_argument otherwise.
   */
  Size &operator+=(const Size &element_count) {
    value = get_valid_sum(value, element_count.value);
    return *this;
  }

  /**
   * Multiplies with element count if that results in a valid total element
   * count and throws std::invalid_argument otherwise.
   */
  Size &operator*=(size_type element_count) {
    value = get_valid_product(value, element_count);
    return *this;
  }

  /**
   * Multiplies with element count if that results in a valid total element
   * count and throws std::invalid_argument otherwise.
   */
  Size &operator*=(const Size &element_count) {
    value = get_valid_product(value, element_count.value);
    return *this;
  }

  /**
   * Returns a size with the sum of size and element_count if that results in a
   * valid total element count and throws std::invalid_argument otherwise.
   */
  friend Size operator+(Size size, size_type element_count) {
    size += element_count;
    return size;
  }

  /**
   * Returns a size with the sum of size and element_count if that results in a
   * valid total element count and throws std::invalid_argument otherwise.
   */
  friend Size operator+(Size size, const Size &element_count) {
    size += element_count;
    return size;
  }

  /**
   * Returns a size with the product of size and element_count if that results
   * in a valid total element count and throws std::invalid_argument otherwise.
   */
  friend Size operator*(Size size, size_type element_count) {
    size *= element_count;
    return size;
  }

  /**
   * Returns a size with the product of size and element_count if that results
   * in a valid total element count and throws std::invalid_argument otherwise.
   */
  friend Size operator*(Size size, const Size &element_count) {
    size *= element_count;
    return size;
  }

  /**
   * Assigns a new element count is that is valid and throws
   * std::invalid_argument otherwise.
   */
  Size &operator=(size_type element_count) {
    value = get_valid_size(element_count);
    return *this;
  }

  /**
   * Explicitly assigns an element_count with different typing if that results
   * in a valid element_count and throws std::invalid_argument otherwise.
   */
  template <typename ST, ST tmax>
  void assign(const Size<element_size, ST, tmax> &size) {
    value = get_assignable_size(size);
    return *this;
  }

private:
  size_type value;
};

template <typename T, typename size_type = std::size_t,
          size_type max_size_bits = sizeof(size_type) * 8>
using SizeFor = Size<sizeof(T), size_type, max_size_bits>;

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_SIZE_H
