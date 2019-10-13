#ifndef SIMPLE_DSP_ALIGNMENT_H
#define SIMPLE_DSP_ALIGNMENT_H
/*
 * simple-dsp/alignment.h
 *
 * Added by michel on 2019-09-10
 * Copyright (C) 2015-2019 Michel Fleur.
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
#include <simple-dsp/attributes.h>
#include <simple-dsp/power2.h>
#include <simple-dsp/addressing.h>

/**
 * The C++20 standard is going to include a template that makes the compiler assume
 * that a certain pointer is aligned to a certain number of bytes. This is described
 * in http://open-std.org/JTC1/SC22/WG21/docs/papers/2018/p1007r2.pdf. This document
 * contains an example to implement this assumption, using using current C++17
 * compilers.
 */
#if __cplusplus <= 201703L
/*
 * It is *assumed* here that C++20 will use something higher than 201703. If this assumption
 * proves false that will be amended.
 */
template<std::size_t N, typename T>
sdsp_nodiscard sdsp_force_inline constexpr T* assume_aligned(T* ptr) {
#if defined(__clang__) || (defined(__GNUC__) && !defined(__ICC))
  return reinterpret_cast<T*>(__builtin_assume_aligned(ptr, N));

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

  namespace base {
    namespace {

      constexpr size_t MAX_ALIGNMENT = 16384;

      template<typename T, size_t ALIGNMENT>
      struct BaseAlignedMetric {

        static_assert(
                Power2Const::is(ALIGNMENT) && ALIGNMENT <= MAX_ALIGNMENT,
                "Alignment must be a power of 2 that is not larger than 16384.");

        static_assert(
                (ALIGNMENT >= sizeof(T)) && (ALIGNMENT % sizeof(T) == 0),
                "Alignment must be a multiple of the type's size.'");

        static constexpr size_t alignment = ALIGNMENT;
        static constexpr size_t elementSize = sizeof(T);
        static constexpr size_t alignmentElements = ALIGNMENT / sizeof(T);
        static constexpr size_t maximumElements = Size<T>::maximum;
        static constexpr size_t maximumFrames = maximumElements / alignmentElements;

        using type = T;

        sdsp_nodiscard sdsp_force_inline static T* assumeAligned(T* p) {
          return ::assume_aligned<ALIGNMENT, T>(p);
        }

        sdsp_nodiscard sdsp_force_inline static const T* assumeAligned(const T* p) {
          return ::assume_aligned<ALIGNMENT, T>(p);
        }

        /**
         * Tests whether the indicated number is aligned.
         * @param number The number to be tested, e.g. a count or offset.
         * @return {@code true} is the number is aligned
         */
        sdsp_nodiscard static bool isAligned(size_t number) {
          return (number & (ALIGNMENT - 1)) == 0;
        }

        /**
         * Tests whether the indicated pointer is aligned.
         * @param number The pointer to be tested.
         * @return {@code true} is the number is a multiple.
         */
        sdsp_nodiscard static bool isAlignedPtr(const T* ptr) {
          return isAligned(reinterpret_cast<size_t>(ptr));
        }

        /**
         * Tests whether the indicated number is a multiple of the number of elements
         * in an aligned block.
         * @param number The number to be tested, e.g. a count or offset.
         * @return {@code true} is the number is a multiple.
         */
        sdsp_nodiscard static bool isAlignedNumberOfElements(size_t number) {
          return number % alignmentElements == 0;
        }

        sdsp_nodiscard static T* verifiedAligned(T* ptr) {
          if (isAlignedPtr(ptr)) {
            return ptr;
          }
          throw std::invalid_argument("AlignedMetric: Pointer is not properly aligned");
        }

        sdsp_nodiscard static const T* verifiedAligned(const T* ptr) {
          if (isAlignedPtr(ptr)) {
            return ptr;
          }
          throw std::invalid_argument("AlignedMetric: Pointer is not properly aligned");
        }

      };

      template<typename T>
      struct BaseAlignedMetric<T, 0> {

        static constexpr size_t alignment = 0;
        static constexpr size_t elementSize = sizeof(T);
        static constexpr size_t alignmentElements = 1;
        static constexpr size_t maximumElements = Size<T>::maximum;
        static constexpr size_t maximumFrames = maximumElements;
        using type = T;

        sdsp_nodiscard sdsp_force_inline static T* assumeAligned(T* p) {
          return p;
        }

        sdsp_nodiscard sdsp_force_inline static const T* assumeAligned(const T* p) {
          return p;
        }

        sdsp_nodiscard static bool isAligned(size_t) { return true; }

        sdsp_nodiscard static bool isAlignedPtr(const T*) { return true; }

        sdsp_nodiscard static bool isAlignedNumberOfElements(size_t) { return true; }

        sdsp_nodiscard static T* verifiedAligned(T* ptr) { return ptr; }

        sdsp_nodiscard static const T* verifiedAligned(const T* ptr) { return ptr; }
      };

      sdsp_nodiscard static constexpr size_t effectiveAlignment(size_t alignment) {
        return alignment > 1
               ? alignment
               : 0;
      }

    } // namespace anonymous
  } // namespace simpledsp::util::base

} // namespace
// simpledsp::align


namespace simpledsp {

  template<typename T, size_t ALIGNMENT>
  using AlignedMetric = base::BaseAlignedMetric<T, base::effectiveAlignment(ALIGNMENT)>;

  template<std::size_t N, typename T>
  sdsp_nodiscard sdsp_force_inline constexpr T* assumeAligned(T* ptr) {
    return AlignedMetric<T, N>::assumeAligned(ptr);
  }

  template<std::size_t N, typename T>
  sdsp_nodiscard sdsp_force_inline constexpr const T* assumeAligned(const T* ptr) {
    return AlignedMetric<T, N>::assumeAligned(ptr);
  }

} // namespace simpledsp::util

#endif //SIMPLE_DSP_ALIGNMENT_H
