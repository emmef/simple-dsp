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

static_assert(stolen_address_bits >= 0,
              "Size: stolen_address_bits cannot be negative.");
static_assert(stolen_address_bits < sizeof(size_t) * 8 - 7,
              "Size: stolen address_bits must leave at least 8 bits of "
              "addressing space.");

static constexpr int effective_size_t_bits =
    sizeof(size_t) * 8 - stolen_address_bits;

template <typename size_type>
static constexpr bool size_is_valid(size_type size, size_t size_max) noexcept {
  return size > 0 && size <= size_max;
}

template <typename size_type>
static constexpr bool size_is_valid_index(size_type index,
                                          size_t index_max) noexcept {
  return index <= index_max;
}

template <typename dst_size_type, typename src_size_type>
static constexpr bool size_can_assign(src_size_type size,
                                      dst_size_type size_max) noexcept {
  static_assert(
      sizeof(dst_size_type) <= sizeof(size_t),
      "size_can_assing: dst_size_type cannot be larger than std::size_t.");
  static_assert(
      sizeof(src_size_type) <= sizeof(size_t),
      "size_can_assign: src_size_type cannot be larger than std::size_t.");

  return size > 0 && size_t(size) <= size_t(size_max);
}

template <typename size_type>
static constexpr bool size_is_valid_sum(size_type v1, size_type v2,
                                        size_type size_max) noexcept {
  return v1 > 0 ? v1 <= size_max && size_max - v1 >= v2
                : v2 > 0 && v2 <= size_max;
}

template <typename size_type>
static constexpr bool size_is_valid_product(size_type v1, size_type v2,
                                            size_t size_max) noexcept {
  return v1 > 0 && v2 > 0 && size_max / v1 >= v2;
}

namespace internal {
namespace {
template <typename size_type>
static constexpr size_type max_bit_mask_from_max_index(size_type max_index) {
  return PowerTwo<size_type>::is_minus_one(max_index)
             ? max_index
             : Bits<size_type>::fill(max_index) >> 1;
};
} // namespace
} // namespace internal

template <typename T = char, typename SIZE = size_t,
          int max_size_bits = sizeof(SIZE) * 8>
struct Size;

template <typename SIZE = size_t, int size_bit_limit = sizeof(SIZE) * 8>
struct SizeType {

  using size_type = SIZE;

  template <typename Element>
  using Size = Size<Element, size_type, size_bit_limit>;

  static_assert(
      std::is_integral<size_type>::value,
      "simpledsp::size::system::SizeType: size_type must be an integral type.");

  static_assert(
      !std::is_signed<size_type>::value,
      "simpledsp::size::system::SizeType: size_type must be an unsigned type.");

  static_assert(sizeof(size_type) <= sizeof(size_t),
                "simpledsp::size::system::SizeType: size_type cannot be larger "
                "than std::size_t.");

  static_assert(size_bit_limit > 7,
                "SizeType: explicit size_bit_limit must be at least 7.");

  static_assert(size_bit_limit <= sizeof(size_type) * 8,
                "SizeType: explicit size_bit_limit cannot exceed number of "
                "bits in size_type.");

  static constexpr int type_bits = 8 * sizeof(size_type);

  static constexpr int size_bits =
      std::min(size_bit_limit, effective_size_t_bits);

  static constexpr size_type max = type_bits == size_bits
                                       ? std::numeric_limits<size_type>::max()
                                       : size_type(1) << size_bits;

  static constexpr size_type max_index =
      type_bits == size_bits ? std::numeric_limits<size_type>::max()
                             : (size_type(1) << size_bits) - 1;

  static constexpr size_type max_bit_mask =
      internal::max_bit_mask_from_max_index(max_index);

  static constexpr size_type max_count_for_element_size(size_t element_size) {
    return max / std::max(size_t(1), element_size);
  }

  static constexpr size_type max_index_for_element_size(size_t element_size) {
    return element_size > 1 ? max_count_for_element_size(element_size) - 1
                            : max_index;
  }

  static constexpr size_type
  max_bit_mask_for_element_size(size_t element_size) {
    return internal::max_bit_mask_from_max_index(
        max_index_for_element_size(element_size));
  }

  static constexpr bool is_valid(size_type size) noexcept {
    return size_is_valid(size, max);
  }

  static constexpr bool is_valid_index(size_type index) noexcept {
    return size_is_valid_index(index, max_index);
  }

  template <typename src_size_type>
  static constexpr bool can_assign(src_size_type size) noexcept {
    return size_can_assign(size, max);
  }

  static constexpr bool is_valid_sum(size_type v1, size_type v2) noexcept {
    return size_is_valid_sum(v1, v2, max);
  }

  static constexpr bool is_valid_product(size_type v1, size_type v2) noexcept {
    return size_is_valid_product(v1, v2, max);
  }
};

static constexpr size_t system_max_size_in_bytes = SizeType<size_t>::max;

static constexpr size_t system_max_byte_index = SizeType<size_t>::max_index;

template <typename T, typename SIZE, int max_size_bits> struct Size {
  using size_type = SIZE;
  using Type = SizeType<size_type, max_size_bits>;

  static constexpr size_type max = Type::max_count_for_element_size(sizeof(T));
  static constexpr size_type max_index =
      Type::max_index_for_element_size(sizeof(T));
  static constexpr size_type max_bit_mask =
      Type::max_bit_mask_for_element_size(max_index);

  sdsp_nodiscard static constexpr bool is_valid(size_type size) noexcept {
    return size_is_valid(size, max);
  }

  sdsp_nodiscard static constexpr bool
  is_valid_index(size_type index) noexcept {
    return size_is_valid_index(index, max_index);
  }

  sdsp_nodiscard static constexpr bool is_valid_sum(size_type v1,
                                                    size_type v2) noexcept {
    return size_is_valid_sum(v1, v2, max);
  }

  sdsp_nodiscard static constexpr bool is_valid_product(size_type v1,
                                                        size_type v2) noexcept {
    return size_is_valid_product(v1, v2, max);
  }

  template <typename src_size_type>
  sdsp_nodiscard static constexpr bool can_assign(src_size_type size) noexcept {
    return size_can_assign(size, max);
  }

  template <typename S, typename ST, ST tmax>
  sdsp_nodiscard static constexpr bool
  can_assign(const Size<S, ST, tmax> &size) {
    return can_assign(size.value);
  }

  sdsp_nodiscard static constexpr size_type get_valid_size(size_type size) {
    if (is_valid(size)) {
      return size;
    }
    throw std::invalid_argument(
        "Size: size must be positive and not greater than Size::max.");
  }

  sdsp_nodiscard static constexpr size_type get_valid_index(size_type index) {
    if (is_valid_index(index)) {
      return index;
    }
    throw std::invalid_argument(
        "Size: index must not be greater than Size::max_index");
  }

  sdsp_nodiscard static constexpr size_type get_valid_sum(size_type v1,
                                                          size_type v2) {
    if (is_valid_sum(v1, v2)) {
      return v1 + v2;
    }
    throw std::invalid_argument(
        "Size: sum must be positive and not greater than Size::max.");
  }

  sdsp_nodiscard static constexpr size_type get_valid_product(size_type v1,
                                                              size_type v2) {
    if (is_valid_product(v1, v2)) {
      return v1 * v2;
    }
    throw std::invalid_argument(
        "Size: product must be positive and not greater than Size::max.");
  }

  template <typename src_size_type>
  sdsp_nodiscard static constexpr size_type
  get_assignable_size(src_size_type size) {
    if (can_assign(size)) {
      return size;
    }
    throw std::invalid_argument(
        "Size: value is zero or too large to be assingned.");
  }

  template <typename S, typename ST, ST tmax>
  sdsp_nodiscard static constexpr size_type
  get_assignable_size(const Size<S, ST, tmax> &size) {
    if (can_assign(size)) {
      return size;
    }
    throw std::invalid_argument(
        "Size: value is zero or too large to be assingned.");
  }

  Size(const size_t size) : value(get_valid_size(size)) {}

  Size(const Size &size) : value(size.value) {}

  Size(const Size &&size) : value(size.value) {}

  template <typename ST, ST tmax>
  Size(const Size<T, ST, tmax> &size) : value(get_compatible_size(size)) {}

  template <typename ST, ST tmax>
  Size(const Size<T, ST, tmax> &&size) : value(get_compatible_size(size)) {}

  Size &operator=(size_t size) {
    value = get_valid_size(size);
    return *this;
  }

  Size &operator=(const Size &size) {
    value = size.value;
    return *this;
  }

  template <typename ST, ST tmax>
  Size &operator=(const Size<T, ST, tmax> &size) {
    value = get_compatible_size(size);
    return *this;
  }

  sdsp_nodiscard operator size_type() const noexcept { return value; }

  sdsp_nodiscard size_t operator()() const noexcept { return value; }

  sdsp_nodiscard size_t maximum() const noexcept { return max; }

  sdsp_nodiscard size_t maximum_index() const noexcept { return max_index; }

  Size &operator+=(size_type v) {
    value = get_valid_sum(value, v);
    return *this;
  }

  Size &operator*=(size_type v) {
    value = get_valid_product(value, v);
    return *this;
  }

  Size operator+(size_type v) const {
    Size result(*this);
    result += v;
    return result;
  }

  Size operator*(size_type v) const {
    Size result(*this);
    result *= v;
    return result;
  }

private:
  size_type value;
};

template <typename T, typename S, S M>
static Size<T, S, M> operator+(S v1, const Size<T, S, M> &v2) {
  return v2 + v1;
}

template <typename T, typename S, S M>
static Size<T, S, M> operator+(const Size<T, S, M> &v1,
                               const Size<T, S, M> &v2) {
  return v1 + (S)v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> &&operator+(Size<T, S, M> &&v1, const Size<T, S, M> &v2) {
  v1 += v2;
  return v1;
}

template <typename T, typename S, S M>
static Size<T, S, M> &&operator+(const Size<T, S, M> &v1, Size<T, S, M> &&v2) {
  v2 += v1;
  return v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> &&operator+(const Size<T, S, M> &&v1, Size<T, S, M> &&v2) {
  v2 += v1;
  return v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> operator*(S v1, const Size<T, S, M> &v2) {
  return v2 * v1;
}

template <typename T, typename S, S M>
static Size<T, S, M> operator*(const Size<T, S, M> &v1,
                               const Size<T, S, M> &v2) {
  return v1 * (S)v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> &&operator*(Size<T, S, M> &&v1, const Size<T, S, M> &v2) {
  v1 *= v2;
  return v1;
}

template <typename T, typename S, S M>
static Size<T, S, M> &&operator*(const Size<T, S, M> &v1, Size<T, S, M> &&v2) {
  v2 *= v1;
  return v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> &&operator*(const Size<T, S, M> &&v1, Size<T, S, M> &&v2) {
  v2 *= v1;
  return v2;
}

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_SIZE_H
