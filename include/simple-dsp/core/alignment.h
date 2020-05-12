#ifndef SIMPLE_DSP_CORE_ALIGNMENT_H
#define SIMPLE_DSP_CORE_ALIGNMENT_H
/*
 * simple-dsp/core/alignment.h
 *
 * Added by michel on 2019-09-10
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
#include <simple-dsp/core/attributes.h>
#include <simple-dsp/core/power2.h>

/**
 * The C++20 standard is going to include a template that makes the compiler
 * assume that a certain pointer is aligned to a certain number of bytes. This
 * is described in
 * http://open-std.org/JTC1/SC22/WG21/docs/papers/2018/p1007r2.pdf. This
 * document contains an example to implement this assumption, using using
 * current C++17 compilers.
 */
#if __cplusplus <= 201703L
/*
 * It is *assumed* here that C++20 will use something higher than 201703. If
 * this assumption proves false that will be amended.
 */
template <std::size_t N, typename T>
sdsp_nodiscard sdsp_force_inline constexpr T *assume_aligned(T *ptr) {
#if defined(__clang__) || (defined(__GNUC__) && !defined(__ICC))
  return reinterpret_cast<T *>(__builtin_assume_aligned(ptr, N));

#elif defined(_MSC_VER)
  if ((reinterpret_cast<std::uintptr_t>(ptr) & ((1 << N) - 1)) == 0)
    return ptr;
  else
    __assume(0);
#elif defined(__ICC)
  if (simpledsp::algorithm::Power2::constant::is(N)) {
    __assume_aligned(ptr, N);
  }
  return ptr;
#else
  // Unknown compiler — do nothing
  return ptr;
#endif
}

#endif //  __cplusplus <= 201703L

namespace simpledsp {

enum class SimdAlignType { BYTES, ELEMENTS };

namespace internal {

template <typename T> struct InternalVectorBaseTypeCheck {
  static_assert(std::is_floating_point<T>::value || std::is_integral<T>::value,
                "Alignment base should be a floating point or integral type.");

  using type = T;
  static constexpr size_t element_size = sizeof(T);
};

template <typename T, SimdAlignType type, size_t count>
struct InternalVectorBase;

template <typename T, size_t count>
struct InternalVectorBase<T, SimdAlignType::BYTES, count>
    : public InternalVectorBaseTypeCheck<T> {
  static_assert(Power2::is(count));

  using InternalVectorBaseTypeCheck<T>::element_size;
  static constexpr size_t _count =
      count == 1 ? count : std::max(count, sizeof(size_t));

  static constexpr size_t elements = std::max(size_t(1), _count / element_size);
  static constexpr size_t bytes = _count;
  static constexpr size_t mask = bytes - 1;
};

template <typename T, size_t count>
struct InternalVectorBase<T, SimdAlignType::ELEMENTS, count>
    : public InternalVectorBaseTypeCheck<T> {
  static_assert(Power2::is(count));

  using InternalVectorBaseTypeCheck<T>::element_size;
  static constexpr size_t elements = count;
  static constexpr size_t bytes =
      std::max(count * element_size, sizeof(size_t));
  static constexpr size_t mask = bytes - 1;
};

template <typename T, SimdAlignType TYPE>
struct InternalVectorBase<T, TYPE, size_t(0)>
    : public InternalVectorBaseTypeCheck<T> {

  using InternalVectorBaseTypeCheck<T>::element_size;
  static constexpr size_t elements = 1;
  static constexpr size_t bytes = alignof(T);
  static constexpr size_t mask = bytes - 1;
};

template <size_t bytes> struct AssumeAligned {

  template <typename T>
  sdsp_nodiscard sdsp_force_inline static T *aligned(T *p) {
    return ::assume_aligned<bytes, T>(p);
  }

  template <typename T>
  sdsp_nodiscard sdsp_force_inline static const T *aligned(const T *p) {
    return ::assume_aligned<bytes, T>(p);
  }
};

template <> struct AssumeAligned<0> {
  template <typename T>
  sdsp_nodiscard sdsp_force_inline static T *aligned(T *p) {
    return p;
  }

  template <typename T>
  sdsp_nodiscard sdsp_force_inline static const T *aligned(const T *p) {
    return p;
  }
};

template <typename T>
using AlignmentBase =
#ifdef SIMPLE_CORE_SIMD_ALIGNMENT_COUNT
#if SIMPLE_CORE_SIMD_ALIGNMENT_TYPE == 1
    internal::InternalVectorBase<SimdAlignType::BYTES,
                                 (SIMPLE_CORE_SIMD_ALIGNMENT_COUNT)>;
#elif SIMPLE_CORE_SIMD_ALIGNMENT_TYPE == 2
    internal::InternalVectorBase<T, SimdAlignType::ELEMENTS,
                                 (SIMPLE_CORE_SIMD_ALIGNMENT_COUNT)>;
#else
#error Invalid value for SIMPLE_CORE_SIMD_ALIGNMENT_TYPE specified: must be 1 (bytes) or 2 (elements).
#endif
#else
    /**
     * This is a safe cover-all guess on 64-bit x86-64 systems: any suggestions
     * for improved guesses are welcome.
     */
    internal::InternalVectorBase<T, SimdAlignType::ELEMENTS, 4>;
#endif

}; // namespace internal

/**
 * Returns the value if it is already a multiple of power_of_two or the
 * next multiple of power_of_two otherwise. If power_of_two is not a power
 * of two, the alignment is done with the next higher power of two.
 *
 * @param value Value to be aligned
 * @param power_of_two The power of two to align to, or the next bigger
 * power of two.
 * @return the aligned value
 */
template <typename size_type>
static constexpr size_type get_aligned_with(size_type value,
                                            size_type alignment) {
  size_type filled = Bits<size_type>::fill(alignment >> 1);
  return (value + filled) & ~filled;
}

template <typename size_type>
static constexpr bool is_aligned_with(const size_type value,
                                      const size_type power_of_two) {
  return value == get_aligned_with(value, power_of_two);
}

template <typename T> struct AlignedFor : internal::AlignmentBase<T> {
  using Base = internal::AlignmentBase<T>;
  using Base::bytes;
  using Base::element_size;
  using Base::elements;
  using Base::mask;
  using Base::type;

  /**
   * Tests whether the indicated number is aligned.
   * @param number The number to be tested, e.g. a count or offset.
   * @return {@code true} is the number is aligned
   */
  sdsp_nodiscard static bool is(size_t number) {
    return (number & (mask)) == 0;
  }

  /**
   * Tests whether the indicated pointer is aligned.
   * @param number The pointer to be tested.
   * @return {@code true} is the number is a multiple.
   */
  sdsp_nodiscard static bool is(const T *ptr) {
    return is(reinterpret_cast<size_t>(ptr));
  }

  sdsp_nodiscard sdsp_force_inline static T *aligned(T *p) {
    return internal::AssumeAligned<bytes>::assume_aligned(p);
  }

  sdsp_nodiscard sdsp_force_inline static const T *aligned(const T *p) {
    return internal::AssumeAligned<bytes>::assume_aligned(p);
  }

  sdsp_nodiscard static T *verified(T *ptr) {
    if (is(ptr)) {
      return ptr;
    }
    throw std::invalid_argument("Aligned: Pointer is not properly aligned");
  }

  sdsp_nodiscard static const T *verified(const T *ptr) {
    if (is(ptr)) {
      return ptr;
    }
    throw std::invalid_argument("Aligned: Pointer is not properly aligned");
  }

  sdsp_nodiscard sdsp_force_inline static T *verified_aligned(T *ptr) {
    return aligned(verified(ptr));
  }

  sdsp_nodiscard sdsp_force_inline static const T *
  verified_aligned(const T *ptr) {
    return aligned(verified(ptr));
  }
};

struct Aligned {

  template <typename T> sdsp_nodiscard static bool is(const T *ptr) {
    return AlignedFor<T>::is(ptr);
  }

  template <typename T>
  sdsp_nodiscard sdsp_force_inline static T *aligned(T *p) {
    return AlignedFor<T>::aligned(p);
  }

  template <typename T>
  sdsp_nodiscard sdsp_force_inline static const T *aligned(const T *p) {
    return AlignedFor<T>::aligned(p);
  }

  template <typename T> sdsp_nodiscard static T *verified(T *ptr) {
    return AlignedFor<T>::verified(ptr);
  }

  template <typename T> sdsp_nodiscard static const T *verified(const T *ptr) {
    return AlignedFor<T>::verified(ptr);
  }

  template <typename T>
  sdsp_nodiscard sdsp_force_inline static T *verified_aligned(T *ptr) {
    return AlignedFor<T>::verified_aligned(ptr);
  }

  template <typename T>
  sdsp_nodiscard sdsp_force_inline static const T *
  verified_aligned(const T *ptr) {
    return AlignedFor<T>::verified_aligned(ptr);
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_ALIGNMENT_H
