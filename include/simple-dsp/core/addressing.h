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
#include <algorithm>
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
static constexpr size_t system_address_stolen_bits =
#if SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS > 0
    std::clamp(SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS,
               sizeof(size_t) < 4 ? 0 : 1, sizeof(size_t) * 8 - 7);
#else
    sizeof(size_t) <= 4 ? 1 : 1 + 8 * (sizeof(size_t) / 4);
#endif
static constexpr size_t system_max_size_in_bytes =
    (std::numeric_limits<size_t>::max()
     << internal::system_address_stolen_bits) +
    (internal::system_address_stolen_bits > 1 ? 1 : 0);

template <typename SIZE_TYPE>
static constexpr SIZE_TYPE system_max_size_type_bytes() noexcept {
  return std::min((size_t)std::numeric_limits<SIZE_TYPE>::max(),
                  system_max_size_in_bytes);
}

} // end namespace internal

static constexpr size_t max_size_in_bytes = internal::system_max_size_in_bytes;

template <typename T = char, typename SIZE_TYPE = size_t,
          SIZE_TYPE max_override = 0>
struct Elements {
  static_assert(std::is_integral<SIZE_TYPE>::value &&
                    std::is_unsigned<SIZE_TYPE>::value,
                "Size type must be an unsigned integral");

  using size_type = SIZE_TYPE;

  struct Size {
    static constexpr size_type max = std::min(
        (internal::system_max_size_type_bytes<SIZE_TYPE>() - alignof(T)) /
            sizeof(T),
        max_override > 0 ? max_override
                         : internal::system_max_size_type_bytes<SIZE_TYPE>());

    static constexpr bool is_valid(size_type value) noexcept {
      return value > 0 && value <= max;
    }

    static constexpr size_type get_value_if_valid(
        size_type value,
        const char *msg =
            "ElementCount::Size: value too large or not positive") {
      if (is_valid(value)) {
        return value;
      }
      throw std::invalid_argument(msg);
    }

    static constexpr bool is_valid_sum(size_type v1, size_type v2) noexcept {
      return v1 <= max && max - v1 >= v2;
    }

    static constexpr bool sum_both_valid(size_type v1, size_type v2) noexcept {
      return v1 > 0 && v2 > 0 && sum_within(v1, v2);
    }

    static constexpr size_type get_sum_if_valid(size_type v1, size_type v2,
                                                const char *const msg) {
      if (is_valid_sum(v1, v2)) {
        return v1 + v2;
      }
      throw std::invalid_argument(msg);
    }

    static constexpr size_type sum_if_both_valid(size_type v1, size_type v2,
                                                 const char *const msg) {
      if (sum_both_valid(v1, v2)) {
        return v1 + v2;
      }
      throw std::invalid_argument(msg);
    }

    static constexpr size_type get_value_if_valid_sum(size_type value,
                                                      size_type other,
                                                      const char *const msg) {
      if (is_valid_sum(value, other)) {
        return value;
      }
      throw std::invalid_argument(msg);
    }

    static constexpr size_type value_if_valid_sum_both(size_type value,
                                                       size_type other,
                                                       const char *const msg) {
      if (sum_both_valid(value, other)) {
        return value;
      }
      throw std::invalid_argument(msg);
    }

    static constexpr bool is_valid_product(size_type v1, size_type v2) {
      return v1 > 0 && v2 > 0 && max / v1 >= v2;
    }

    static constexpr size_type get_product_if_valid(size_type v1, size_type v2,
                                                    const char *const msg) {
      if (is_valid_product(v1, v2)) {
        return v1 * v2;
      }
      throw std::invalid_argument(msg);
    }

    static constexpr size_type
    get_value_if_valid_product(size_type value, size_type other,
                               const char *const msg) {
      if (is_valid_product(value, other)) {
        return value;
      }
      throw std::invalid_argument(msg);
    }
  };

  struct Index {
    static constexpr size_type max = Size::max - 1;

    static constexpr bool is_valid(size_type value) noexcept {
      return value <= max;
    }

    static constexpr size_type get_value_if_valid(
        size_type value,
        const char *msg = "ElementCount::Index: value too large") {
      if (is_valid(value)) {
        return value;
      }
      throw std::invalid_argument(msg);
    }

    static constexpr bool is_valid_sum(size_type v1, size_type v2) noexcept {
      return v1 <= max && max - v1 >= v2;
    }

    static constexpr size_type get_sum_if_valid(size_type v1, size_type v2,
                                                const char *msg) {
      if (is_valid_sum(v1, v2)) {
        return v1 + v2;
      }
      throw std::invalid_argument(msg);
    }

    static constexpr size_type
    get_value_if_valid_sum(size_type value, size_type other, const char *msg) {
      if (is_valid_sum(value, other)) {
        return value;
      }
      throw std::invalid_argument(msg);
    };
  };
};

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
  sdsp_nodiscard static SizeType offset(SizeType o, SizeType maxOffset) {
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
  offset(SizeType o, SizeType maxOffset) noexcept {
    return o % (maxOffset + 1);
  }
};

template <typename SizeType>
struct IndexPolicyBase<SizeType, IndexPolicyType::UNCHECKED> {
  sdsp_nodiscard sdsp_force_inline static SizeType index(SizeType i,
                                                         SizeType) noexcept {
    return i;
  }
  sdsp_nodiscard sdsp_force_inline static SizeType offset(SizeType o,
                                                          SizeType) noexcept {
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

struct Index {
  template <typename S> sdsp_nodiscard static S safe(S i, S size) {
    return Safe<S>::index(i, size);
  }

  template <typename S> sdsp_nodiscard static S unsafe(S i, S size) {
    return Unsafe<S>::index(i, size);
  }
};

struct Offset {
  template <typename S> sdsp_nodiscard static S safe(S i, S size) {
    return Safe<S>::offset(i, size);
  }

  template <typename S> sdsp_nodiscard static S unsafe(S i, S size) {
    return Unsafe<S>::offset(i, size);
  }
};

} // namespace simpledsp::addr

#endif // SIMPLE_DSP_ADDRESSING_H
