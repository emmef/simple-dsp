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
#include <simple-dsp/core/size.h>

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

namespace base {
namespace {

constexpr size_t MAX_ALIGNMENT = 16384;

template <typename T, size_t ALIGNMENT> struct BaseAlignedMetric {

  static_assert(
      PowerTwo<size_t>::is(ALIGNMENT) && ALIGNMENT <= MAX_ALIGNMENT,
      "Alignment must be a power of 2 that is not larger than 16384.");

  static_assert((ALIGNMENT >= sizeof(T)) && (ALIGNMENT % sizeof(T) == 0),
                "Alignment must be a multiple of the type's size.'");

  static constexpr size_t alignment = ALIGNMENT;
  static constexpr size_t alignment_mask = alignment - 1;
  static constexpr size_t elementSize = sizeof(T);
  static constexpr size_t alignmentElements = ALIGNMENT / sizeof(T);
  static constexpr size_t maximumElements = SizeFor<T>::max;
  static constexpr size_t maximumFrames = maximumElements / alignmentElements;

  using type = T;

  sdsp_nodiscard sdsp_force_inline static T *aligned(T *p) {
    return ::assume_aligned<ALIGNMENT, T>(p);
  }

  sdsp_nodiscard sdsp_force_inline static const T *aligned(const T *p) {
    return ::assume_aligned<ALIGNMENT, T>(p);
  }

  /**
   * Tests whether the indicated number is aligned.
   * @param number The number to be tested, e.g. a count or offset.
   * @return {@code true} is the number is aligned
   */
  sdsp_nodiscard static bool is(size_t number) {
    return (number & (alignment_mask)) == 0;
  }

  /**
   * Tests whether the indicated pointer is aligned.
   * @param number The pointer to be tested.
   * @return {@code true} is the number is a multiple.
   */
  sdsp_nodiscard static bool is(const T *ptr) {
    return is(reinterpret_cast<size_t>(ptr));
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
};

template <typename T> struct BaseAlignedMetric<T, 0> {

  static constexpr size_t alignment = 0;
  static constexpr size_t elementSize = sizeof(T);
  static constexpr size_t alignmentElements = 1;
  static constexpr size_t maximumElements = SizeFor<T>::max;
  static constexpr size_t maximumFrames = maximumElements;
  using type = T;

  sdsp_nodiscard sdsp_force_inline static T *aligned(T *p) { return p; }

  sdsp_nodiscard sdsp_force_inline static const T *aligned(const T *p) {
    return p;
  }

  sdsp_nodiscard static bool is(size_t) { return true; }

  sdsp_nodiscard static bool is(const T *) { return true; }

  sdsp_nodiscard static T *verified(T *ptr) { return ptr; }

  sdsp_nodiscard static const T *verified(const T *ptr) { return ptr; }
};

} // anonymous namespace
} // namespace base

template <typename T, size_t ALIGNMENT>
using Aligned = base::BaseAlignedMetric < T,
      ALIGNMENT<2 ? 0 : ALIGNMENT>;

template <std::size_t N, typename T>
sdsp_nodiscard sdsp_force_inline static constexpr T *assume_aligned(T *ptr) {
  return Aligned<T, N>::aligned(ptr);
}

template <std::size_t N, typename T>
sdsp_nodiscard sdsp_force_inline static constexpr const T *
assume_aligned(const T *ptr) {
  return Aligned<T, N>::aligned(ptr);
}

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_ALIGNMENT_H
