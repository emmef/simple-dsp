#ifndef SIMPLE_DSP_ADDRESSING_H
#define SIMPLE_DSP_ADDRESSING_H
/*
 * simple-dsp/util/addressing.h
 *
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
#include "algorithm.h"
#include <algorithm>
#include <simple-dsp/algorithm.h>
#include <simple-dsp/attributes.h>
#include <stdexcept>

namespace simpledsp::addr {

namespace internal {
/**
 * Processes
 * almost never have access to the full address and size ranges that "fits" in
 * size_t. Most hardware and operating systems reserve bits for special
 * purposes, making them inaccessible for the lineair virtual address range.The
 * number of these bits is also such, that you at least have 7 bits of
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
static constexpr int system_address_stolen_bits =
#if SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS > 0
    std::clamp(SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS,
               sizeof(size_t) < 4 ? 0 : 1, sizeof(size_t) * 8 - 7);
#else
    sizeof(size_t) <= 4 ? 1 : 1 + 8 * (sizeof(size_t) / 4);
#endif

static constexpr size_t system_max_size_in_bytes =
    system_address_stolen_bits == 0
        ? std::numeric_limits<size_t>::max()
        : size_t(1) << (sizeof(size_t) * 8 - system_address_stolen_bits);

template <typename SIZE_TYPE> static constexpr bool valid_size_type() {
  return std::is_integral<SIZE_TYPE>::value &&
         std::is_unsigned<SIZE_TYPE>::value;
}

template <typename ELEMENT> static constexpr size_t system_max_elements() {
  return std::max(sizeof(ELEMENT), alignof(ELEMENT)) <= 1
             ? system_max_size_in_bytes
             : (system_max_size_in_bytes - alignof(ELEMENT)) / sizeof(ELEMENT);
}

template <typename ELEMENT, typename SIZE_TYPE>
static constexpr size_t system_max_elements_size_type() {
  static_assert(valid_size_type<SIZE_TYPE>(),
                "Type parameter is not a valid size-type");
  return system_max_elements<ELEMENT>() >>
         (ssize_t(sizeof(size_t)) - ssize_t(sizeof(SIZE_TYPE))) * 8;
}

template <typename S> class SizeLike {
  static_assert(valid_size_type<S>(),
                "Type parameter is not a valid size-type");
};

template <typename S, S limit> class SizeLikeLimited : public SizeLike<S> {
  static_assert(simpledsp::Power2Const::is(limit), "");
};

enum class SizeOperator { ADD, MUL };

} // end namespace internal

template <typename size_type = size_t>
static constexpr bool is_valid_size(size_type size, size_type max) noexcept {
  return size > 0 && size <= max;
}

template <typename dst_size_type, typename src_size_type>
static constexpr bool is_compatible_size(dst_size_type dst_max,
                                         src_size_type size,
                                         src_size_type src_max) noexcept {
  return size_t(src_max) < dst_max || size_t(dst_max) >= size;
}

template <typename size_type = size_t>
static constexpr bool is_valid_size_sum(size_type v1, size_type v2,
                                        size_type max) noexcept {
  return v1 > 0 ? v1 <= max && max - v1 >= v2 : v2 > 0 && v2 <= max;
}

template <typename size_type = size_t>
static constexpr bool is_valid_size_product(size_type v1, size_type v2,
                                            size_type max) noexcept {
  return v1 > 0 && v2 > 0 && max / v1 >= v2;
}

template <typename size_type = size_t>
static constexpr size_type get_valid_size(size_type size, size_type max) {
  if (is_valid_size(size, max)) {
    return size;
  }
  throw std::invalid_argument(
      "Size: size must be positive and not greater than Size::max.");
}

template <typename size_type = size_t>
static constexpr size_type get_valid_size_sum(size_type v1, size_type v2,
                                              size_type max) {
  if (is_valid_size_sum(v1, v2, max)) {
    return v1 + v2;
  }
  throw std::invalid_argument(
      "Size: sum must be positive and not greater than Size::max.");
}

template <typename size_type = size_t>
static constexpr size_type get_valid_size_product(size_type v1, size_type v2,
                                                  size_type max) {
  if (is_valid_size_product(v1, v2, max)) {
    return v1 * v2;
  }
  throw std::invalid_argument(
      "Size: product must be positive and not greater than Size::max.");
}

template <typename dst_size_type, typename src_size_type>
static constexpr dst_size_type get_compatible_size(dst_size_type dst_max,
                                                   src_size_type size,
                                                   src_size_type src_max) {
  if (is_compatible_size(dst_max, size, src_max)) {
    return dst_size_type(size);
  }
  throw std::invalid_argument(
      "Size: size (originating from different size_type) must be positive and "
      "not greater than destination Size::max");
}

static constexpr size_t max_size_in_bytes = internal::system_max_size_in_bytes;

template <typename T = char, typename SIZE_TYPE = size_t,
          SIZE_TYPE max_override = 0>
struct Size {
  static_assert(max_override == 0 || Power2Const::is(max_override),
                "Size: override of maximum size, must be a power of two.");

  using size_type = SIZE_TYPE;

  static constexpr internal::SizeOperator ADD = internal::SizeOperator::ADD;
  static constexpr internal::SizeOperator MUL = internal::SizeOperator::MUL;

  static constexpr size_type max =
      max_override
          ? std::min(size_t(max_override),
                     internal::system_max_elements_size_type<T, SIZE_TYPE>())
          : internal::system_max_elements_size_type<T, SIZE_TYPE>();

  static_assert(
      Power2Const::is(max) || Power2Const::is_minus_one(max),
      "Size: Size::max must be a power of two or a power of two minus one");

  static constexpr size_type max_index =
      std::min(sizeof(T), alignof(T)) == 1 && Power2Const::is_minus_one(max)
          ? max
          : max - 1;

  sdsp_nodiscard static constexpr bool is_valid(size_type size) noexcept {
    return is_valid_size(size, max);
  }

  sdsp_nodiscard static constexpr bool
  is_valid_index(size_type index) noexcept {
    return index <= max_index;
  }

  sdsp_nodiscard static constexpr bool is_valid_sum(size_type v1,
                                                    size_type v2) noexcept {
    return is_valid_size_sum(v1, v2, max);
  }

  sdsp_nodiscard static constexpr bool is_valid_product(size_type v1,
                                                        size_type v2) noexcept {
    return is_valid_size_product(v1, v2, max);
  }

  template <typename S, typename ST, ST tmax>
  sdsp_nodiscard static constexpr bool
  is_compatible_size(Size<S, ST, tmax> size) {
    return is_compatible_size(max, size.value, Size<S, ST, tmax>::max);
  }

  sdsp_nodiscard static constexpr size_type get_valid_size(size_type size) {
    return simpledsp::addr::get_valid_size(size, max);
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
    return get_valid_size_sum(v1, v2, max);
  }

  sdsp_nodiscard static constexpr size_type get_valid_product(size_type v1,
                                                              size_type v2) {
    return get_valid_size_product(v1, v2, max);
  }

  template <typename ST, ST tmax>
  sdsp_nodiscard static constexpr size_type
  get_compatible_size(Size<T, ST, tmax> size) noexcept {
    return get_compatible_size(max, size.value, Size<T, ST, tmax>::max);
  }

  sdsp_nodiscard static size_type calculate(const internal::SizeOperator &op,
                                            const size_type v1,
                                            const size_type v2) {
    if (op == internal::SizeOperator::ADD) {
      return get_valid_sum(v1, v2);
    } else if (op == internal::SizeOperator::MUL) {
      return get_valid_product(v1, v2);
    }
    throw std::invalid_argument("Size: invalid operator specified");
  }

  size_type value;

  Size(const size_t size) : value(get_valid_size(size)) {}

  Size(const Size &size) : value(size.value) {}

  Size(const Size &&size) : value(size.value) {}

  template <typename ST, ST tmax>
  Size(const Size<T, ST, tmax> &size) : value(get_compatible_size(size)) {}

  template <typename ST, ST tmax>
  Size(const Size<T, ST, tmax> &&size) : value(get_compatible_size(size)) {}

  Size(internal::SizeOperator op, size_type v1, size_type v2)
      : value(calculate(op, v1, v2)) {}

  Size &operator=(size_t size) { value = get_valid_size(size); }

  Size &operator=(const Size &size) { value = size.value; }

  template <typename ST, ST tmax>
  Size &operator=(const Size<T, ST, tmax> &size) {
    value = get_compatible_size(size);
  }

  sdsp_nodiscard operator size_type() const noexcept { return value; }

  sdsp_nodiscard size_t maximum() const noexcept { return max; }

  sdsp_nodiscard size_t maximum_index() const noexcept { return max_index; }

  sdsp_nodiscard Size operator+(size_type v) const {
    return get_valid_sum(value, v);
  }

  sdsp_nodiscard Size operator*(size_type v) const {
    return get_valid_product(value, v);
  }

  Size &operator+=(size_type v) {
    value = get_valid_sum(value, v);
    return *this;
  }

  Size &operator*=(size_type v) {
    value = get_valid_product(value, v);
    return *this;
  }
};

template <typename T, typename S, S M>
static Size<T, S, M> operator+(S v1, const Size<T, S, M> &v2) {
  return v2 + v1;
}

template <typename T, typename S, S M>
static Size<T, S, M> operator+(const Size<T, S, M> &v1,
                               const Size<T, S, M> &v2) {
  return v1 + v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> &operator+(Size<T, S, M> &&v1, const Size<T, S, M> &v2) {
  v1 += v2;
  return v1;
}

template <typename T, typename S, S M>
static Size<T, S, M> &operator+(const Size<T, S, M> &v1, Size<T, S, M> &&v2) {
  v2 += v1;
  return v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> &operator+(Size<T, S, M> &&v1, Size<T, S, M> &&v2) {
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
  return v1 * v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> &operator*(Size<T, S, M> &&v1, const Size<T, S, M> &v2) {
  v1 *= v2;
  return v1;
}

template <typename T, typename S, S M>
static Size<T, S, M> &operator*(const Size<T, S, M> &v1, Size<T, S, M> &&v2) {
  v2 *= v1;
  return v2;
}

template <typename T, typename S, S M>
static Size<T, S, M> &operator*(Size<T, S, M> &&v1, Size<T, S, M> &&v2) {
  v2 *= v1;
  return v2;
}

enum class IndexPolicyType { THROW, WRAP, UNCHECKED };

template <typename SizeType, IndexPolicyType type> struct IndexPolicyBase;

template <typename SizeType>
struct IndexPolicyBase<SizeType, IndexPolicyType::THROW> {
  sdsp_nodiscard static SizeType index(SizeType i, SizeType size) {
    if (i < size) {
      return i;
    }
    throw std::invalid_argument("IndexPolicy::index out of range");
  }
  sdsp_nodiscard static SizeType index_incl(SizeType o, SizeType maxOffset) {
    if (o <= maxOffset) {
      return o;
    }
    throw std::invalid_argument("IndexPolicy::offset out of range");
  }
};

template <typename SizeType>
struct IndexPolicyBase<SizeType, IndexPolicyType::WRAP> {
  sdsp_nodiscard sdsp_force_inline static SizeType
  index(SizeType i, SizeType size) noexcept {
    return i % size;
  }
  sdsp_nodiscard sdsp_force_inline static SizeType
  index_incl(SizeType o, SizeType maxOffset) noexcept {
    return o % (maxOffset + 1);
  }
};

template <typename SizeType>
struct IndexPolicyBase<SizeType, IndexPolicyType::UNCHECKED> {
  sdsp_nodiscard sdsp_force_inline static SizeType index(SizeType i,
                                                         SizeType) noexcept {
    return i;
  }
  sdsp_nodiscard sdsp_force_inline static SizeType
  index_incl(SizeType o, SizeType) noexcept {
    return o;
  }
};

template <typename S = size_t>
using Safe =
#ifndef SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED
    IndexPolicyBase<S, IndexPolicyType::THROW>;
#else
    IndexPolicyBase<S, IndexPolicyType::UNCHECKED>;
#endif

template <typename S = size_t>
using Unsafe =
#ifndef SDSP_INDEX_POLICY_FORCE_UNSAFE_CHECKED
    IndexPolicyBase<S, IndexPolicyType::UNCHECKED>;
#else
    IndexPolicyBase<S, IndexPolicyType::THROW>;
#endif

template <typename S = size_t>
using Throw = IndexPolicyBase<S, IndexPolicyType::THROW>;

template <typename S = size_t>
using Wrap = IndexPolicyBase<S, IndexPolicyType::WRAP>;

template <typename S = size_t>
using Unchecked = IndexPolicyBase<S, IndexPolicyType::UNCHECKED>;

template <typename S> sdsp_nodiscard static S checked_index(S i, S size) {
  return Throw<S>::index(i, size);
}

template <typename S> sdsp_nodiscard static S safe_index(S i, S size) {
  return Safe<S>::index(i, size);
}

template <typename S> sdsp_nodiscard static S unsafe_index(S i, S size) {
  return Unsafe<S>::index(i, size);
}

template <typename S> sdsp_nodiscard static S checked_index_incl(S i, S size) {
  return Throw<S>::index_incl(i, size);
}

template <typename S> sdsp_nodiscard static S safe_index_incl(S i, S size) {
  return Safe<S>::index_incl(i, size);
}

template <typename S> sdsp_nodiscard static S unsafe_index_incl(S i, S size) {
  return Unsafe<S>::index_incl(i, size);
}

} // namespace simpledsp::addr

#endif // SIMPLE_DSP_ADDRESSING_H
