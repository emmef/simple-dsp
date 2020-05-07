#ifndef SIMPLE_DSP_ARRAYOPS_H
#define SIMPLE_DSP_ARRAYOPS_H
/*
 * simple-dsp/arraycalculus.h
 *
 * Added by michel on 2019-09-11
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
#include <simple-dsp/core/aligneddata.h>
#include <simple-dsp/core/denormal.h>

namespace simpledsp::arrayops {

/*
 * Following functions are forced to be inline if the compiler supports it, so
 * that alignment hints can be passed on to their implementations. Using these
 * methods directly will bypass the compiler and build decisions. All the
 * functions are also included below.
 *
 * IMPORTANT: Functions that modify an array and that involve another array,
 * assume that the data of the arrays DOES NOT OVERLAP.
 *
 */
template <typename T>
sdsp_nodiscard sdsp_force_inline static T inline_sum(const T *data,
                                                     size_t size) {
  T sum = 0;
  for (size_t i = 0; i < size; ++i) {
    sum += data[i];
  }
  return sum;
}

template <typename T>
sdsp_nodiscard sdsp_force_inline static T inline_average(const T *data,
                                                         size_t size) {
  return inline_sum(data, size) / size;
}

template <typename T>
sdsp_nodiscard sdsp_force_inline static T inline_self_product(const T *data,
                                                              size_t size) {
  T sum = 0;
  for (size_t i = 0; i < size; ++i) {
    T x = data[i];
    sum += x * x;
  }
  return sum;
}

template <typename T>
sdsp_nodiscard sdsp_force_inline static T
inline_sum_of_squared_errors(const T *data, size_t size) {
  T average = inline_average(data, size);
  T sum = 0;
  for (size_t i = 0; i < size; ++i) {
    T x = data[i] - average;
    sum += x * x;
  }
  return sum;
}

template <typename T>
sdsp_nodiscard sdsp_force_inline static T
inline_inner_product(const T *v1, const T *v2, size_t size) {
  T sum = 0;
  for (size_t i = 0; i < size; ++i) {
    sum += v1[i] * v2[i];
  }
  return sum;
}

template <typename T>
sdsp_force_inline static void inline_multiply_with(T *v1, T factor,
                                                   size_t size) {
  for (size_t i = 0; i < size; ++i) {
    v1[i] *= factor;
  }
}

template <typename T>
sdsp_force_inline static void inline_multiply_with_range(T *v1, T from, T to,
                                                         size_t size) {
  T factor = from;
  T delta = Denormal<T>::get_flushed((to - from) / (size - 1));
  if (delta != 0) {
    for (size_t i = 0; i < size; ++i, factor += delta) {
      v1[i] *= factor;
    }
  } else {
    inline_multiply_with(v1, from, size);
  }
}

template <typename T>
sdsp_force_inline static void
inline_multiply_with_range_accurate(T *v1, T from, T to, size_t size) {
  T factor = from;
  T delta = Denormal<T>::get_flushed((to - from));
  if (delta != 0) {
    size_t denominator = (size - 1);
    for (size_t i = 0; i < size; ++i) {
      v1[i] *= factor + Denormal<T>::get_flushed((i * delta) / denominator);
    }
  } else {
    inline_multiply_with(v1, from, size);
  }
}

template <typename T>
sdsp_force_inline static void inline_add_to(T *__restrict destination,
                                            const T *__restrict source,
                                            size_t size) {
  for (size_t i = 0; i < size; ++i) {
    destination[i] += source[i];
  }
}

template <typename T>
sdsp_force_inline static void inline_add_to(T *destination, T delta,
                                            size_t size) {
  for (size_t i = 0; i < size; ++i) {
    destination[i] += delta;
  }
}

template <typename T>
sdsp_force_inline static void
inline_add_to_with_factor(T *__restrict destination, const T *__restrict source,
                          T factor, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    destination[i] += source[i] * factor;
  }
}

template <typename T, size_t ALIGNMENT>
sdsp_force_inline static void inline_subtract_from(T *__restrict destination,
                                                   const T *__restrict source,
                                                   size_t size) {
  for (size_t i = 0; i < size; ++i) {
    destination[i] -= source[i];
  }
}

template <typename T>
sdsp_force_inline static void inline_subtract_from(T *destination, T delta,
                                                   size_t size) {
  for (size_t i = 0; i < size; ++i) {
    destination[i] -= delta;
  }
}

template <typename T>
sdsp_force_inline static void
inline_subtract_from_with_factor(T *__restrict destination,
                                 const T *__restrict source, T factor,
                                 size_t size) {
  for (size_t i = 0; i < size; ++i) {
    destination[i] -= source[i] * factor;
  }
}

template <typename T>
static size_t verify_same_sizes(size_t size1, size_t size2) {
  if (size1 == size2) {
    return size1;
  }
  throw std::invalid_argument("verify_same_sizes: sizes are not equal");
}

template <typename T, size_t ALIGNMENT = 0>
sdsp_nodiscard static T sum(const T *data, size_t size) {
  return inline_sum(assume_aligned<ALIGNMENT>(data), size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
sdsp_nodiscard static T sum(const AlignedArray<T, SIZE, ALIGNMENT> &array) {
  return inline_sum(array.ptr()(), SIZE);
}

template <typename T, size_t ALIGNMENT, class C>
sdsp_nodiscard static T sum(const AlignedData<T, ALIGNMENT, C> &array) {
  return inline_sum(array.ptr()(), array.size());
}

/*
 * Calculate the average of an array.
 */

template <typename T, size_t ALIGNMENT = 0>
sdsp_nodiscard static T average(const T *data, size_t size) {
  return inline_average(assume_aligned<ALIGNMENT>(data), size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
sdsp_nodiscard static T average(const AlignedArray<T, SIZE, ALIGNMENT> &array) {
  return inline_average(array.ptr()(), SIZE);
}

template <typename T, size_t ALIGNMENT, class C>
sdsp_nodiscard static T average(const AlignedData<T, ALIGNMENT, C> &array) {
  return inline_average(array.ptr()(), array.size());
}

/*
 * Calculate the inner product of an array with itself (the sum of the squares
 * of all elements).
 */

template <typename T, size_t ALIGNMENT = 0>
sdsp_nodiscard static T self_product(const T *data, size_t size) {
  return inline_self_product(assume_aligned<ALIGNMENT>(data), size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
sdsp_nodiscard static T
self_product(const AlignedArray<T, SIZE, ALIGNMENT> &array) {
  return inline_self_product(array.ptr()(), SIZE);
}

template <typename T, size_t ALIGNMENT, class C>
sdsp_nodiscard static T
self_product(const AlignedData<T, ALIGNMENT, C> &array) {
  return inline_self_product(array.ptr()(), array.size());
}

/*
 * Calculate the sum of the squared differences from the average of an array.
 */

template <typename T, size_t ALIGNMENT>
sdsp_nodiscard static T sum_of_squared_errors(const T *data, size_t size) {
  return inline_sum_of_squared_errors(assume_aligned<ALIGNMENT>(data), size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
sdsp_nodiscard static T
sum_of_squared_errors(const AlignedArray<T, SIZE, ALIGNMENT> &array) {
  return inline_sum_of_squared_errors(array.ptr()(), SIZE);
}

template <typename T, size_t ALIGNMENT, class C>
sdsp_nodiscard static T
sum_of_squared_errors(const AlignedData<T, ALIGNMENT, C> &array) {
  return inline_sum_of_squared_errors(array.ptr()(), array.size());
}

/*
 * Calculate the inner product of two arrays.
 */

template <typename T, size_t ALIGNMENT = 0>
sdsp_nodiscard static T inner_product(const T *v1, const T *v2, size_t size) {
  return inline_inner_product(assume_aligned<ALIGNMENT>(v1),
                              assume_aligned<ALIGNMENT>(v2), size);
}

template <typename T, size_t ALIGNMENT, class C, class D>
sdsp_nodiscard static T
inner_product(const AlignedData<T, ALIGNMENT, C> &array1,
              const AlignedData<T, ALIGNMENT, D> &array2) {
  return inline_inner_product(array1.ptr(), array2.ptr(),
                              ::std::min(array1.size(), array2.size()));
}

template <typename T, size_t ALIGNMENT, class C, class D>
sdsp_nodiscard static T operator*(const AlignedData<T, ALIGNMENT, C> &array1,
                                  const AlignedData<T, ALIGNMENT, D> &array2) {
  return inline_inner_product(array1.ptr(), array2.ptr(),
                              ::std::min(array1.size(), array2.size()));
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
sdsp_nodiscard static T
inner_product(const AlignedArray<T, SIZE, ALIGNMENT> &array1,
              const AlignedArray<T, SIZE, ALIGNMENT> &array2) {
  return inline_inner_product(array1.ptr(), array2.ptr(), SIZE);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
sdsp_nodiscard static T
operator*(const AlignedArray<T, SIZE, ALIGNMENT> &array1,
          const AlignedArray<T, SIZE, ALIGNMENT> &array2) {
  return inline_inner_product(array1.ptr(), array2.ptr(), SIZE);
}

/*
 * Multiply an array with a factor.
 */

template <typename T, size_t ALIGNMENT = 0>
static void multiply_with(T *array, T factor, size_t size) {
  inline_multiply_with(assume_aligned<ALIGNMENT>(array), factor, size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void multiply_with(AlignedArray<T, SIZE, ALIGNMENT> &array, T factor) {
  inline_multiply_with(array.ptr()(), factor, SIZE);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void operator*=(AlignedArray<T, SIZE, ALIGNMENT> &array, T factor) {
  inline_multiply_with(array.ptr()(), factor, SIZE);
}

template <typename T, size_t ALIGNMENT, class C>
static void multiply_with(AlignedData<T, ALIGNMENT, C> &array, T factor) {
  inline_multiply_with(array.ptr()(), factor, array.size());
}

template <typename T, size_t ALIGNMENT, class C>
static void operator*=(AlignedData<T, ALIGNMENT, C> &array, T factor) {
  inline_multiply_with(array.ptr()(), factor, array.size());
}

/*
 * Multiply an array with a factor from one to another value.
 */

template <typename T, size_t ALIGNMENT = 0>
static void multiply_with_range(T *array, T from, T to, size_t size) {
  inline_multiply_with_range(assume_aligned<ALIGNMENT>(array), from, to, size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void multiply_with_range(AlignedArray<T, SIZE, ALIGNMENT> &array, T from,
                                T to) {
  inline_multiply_with_range(array.ptr()(), from, to, SIZE);
}

template <typename T, size_t ALIGNMENT, class C>
static void multiply_with_range(AlignedData<T, ALIGNMENT, C> &array, T from,
                                T to) {
  inline_multiply_with_range(array.ptr()(), from, to, array.size());
}

/*
 * Multiply an array with a factor from one to another value, circumventing
 * accuracy problems.
 */

template <typename T, size_t ALIGNMENT = 0>
static void multiply_with_range_accurate(T *array, T from, T to, size_t size) {
  inline_multiply_with_range_accurate(assume_aligned<ALIGNMENT>(array), from, to,
                                      size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void
multiply_with_range_accurate(AlignedArray<T, SIZE, ALIGNMENT> &array, T from,
                             T to) {
  inline_multiply_with_range_accurate(array.ptr()(), from, to, SIZE);
}

template <typename T, size_t ALIGNMENT, class C>
static void multiply_with_range_accurate(AlignedData<T, ALIGNMENT, C> &array,
                                         T from, T to) {
  inline_multiply_with_range_accurate(array.ptr()(), from, to, array.size());
}

/*
 * Add the source to the destination array.
 */

template <typename T, size_t ALIGNMENT>
static void add_to(T *destination, const T *source, size_t size) {
  inline_add_to(assume_aligned<ALIGNMENT>(destination),
                assume_aligned<ALIGNMENT>(source), size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void add_to(AlignedArray<T, SIZE, ALIGNMENT> &destination,
                   const AlignedArray<T, SIZE, ALIGNMENT> &source) {
  inline_add_to(destination.ptr()(), source.ptr()(), SIZE);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void operator+=(AlignedArray<T, SIZE, ALIGNMENT> &destination,
                       const AlignedArray<T, SIZE, ALIGNMENT> &source) {
  inline_add_to(destination.ptr()(), source.ptr()(), SIZE);
}

template <typename T, size_t ALIGNMENT, class C, class D>
static void add_to(AlignedData<T, ALIGNMENT, C> &destination,
                   const AlignedData<T, ALIGNMENT, D> &source) {
  inline_add_to(destination.ptr()(), source.ptr()(),
                ::std::min(destination.size(), source.size()));
}

template <typename T, size_t ALIGNMENT, class C, class D>
static void add_to_frames_up(AlignedData<T, ALIGNMENT, C> &destination,
                             const AlignedData<T, ALIGNMENT, D> &source,
                             size_t framesUp) {
  inline_add_to(
      destination.frame(framesUp), source.ptr()(),
      ::std::min(
          ::std::max(
              0,
              static_cast<ssize_t>(destination.size()) -
                  framesUp *
                      AlignedData<T, ALIGNMENT, C>::Metric::alignmentElements),
          source.size()));
}

/*
 * Add the source to the destination array with a factor.
 */

template <typename T, size_t ALIGNMENT>
static void add_to_with_factor(T *destination, const T *source, T factor,
                               size_t size) {
  inline_add_to_with_factor(assume_aligned<ALIGNMENT>(destination),
                            assume_aligned<ALIGNMENT>(source), factor, size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void add_to_with_factor(AlignedArray<T, SIZE, ALIGNMENT> &destination,
                               const AlignedArray<T, SIZE, ALIGNMENT> &source,
                               T factor) {
  inline_add_to_with_factor(destination.ptr()(), source.ptr()(), factor, SIZE);
}

template <typename T, size_t ALIGNMENT, class C, class D>
static void add_to_with_factor(AlignedData<T, ALIGNMENT, C> &destination,
                               const AlignedData<T, ALIGNMENT, D> &source,
                               T factor) {
  inline_add_to_with_factor(
      destination.ptr()(), source.ptr()(),
      ::std::min(destination.size(), factor, source.size()));
}

template <typename T, size_t ALIGNMENT, class C, class D>
static void
add_to_with_factor_frames_up(AlignedData<T, ALIGNMENT, C> &destination,
                             const AlignedData<T, ALIGNMENT, D> &source,
                             T factor, size_t framesUp) {
  inline_add_to_with_factor(
      destination.frame(framesUp), source.ptr()(), factor,
      ::std::min(
          ::std::max(
              0,
              static_cast<ssize_t>(destination.size()) -
                  framesUp *
                      AlignedData<T, ALIGNMENT, C>::Metric::alignmentElements),
          source.size()));
}

/*
 * Add a value to all elements of the array.
 */

template <typename T, size_t ALIGNMENT>
static void add_to(T *array, T delta, size_t size) {
  inline_add_to(assume_aligned<ALIGNMENT>(array), delta, size);
}

template <typename T, size_t ALIGNMENT, class C>
static void add_to(AlignedData<T, ALIGNMENT, C> &array, T delta) {
  inline_add_to(array.ptr()(), delta, array.size());
}

template <typename T, size_t ALIGNMENT, class C>
static void operator+=(AlignedData<T, ALIGNMENT, C> &array, T delta) {
  inline_add_to(array.ptr()(), delta, array.size());
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void add_to(AlignedArray<T, SIZE, ALIGNMENT> &array, T delta) {
  inline_add_to(array.ptr()(), delta, SIZE);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void operator+=(AlignedArray<T, SIZE, ALIGNMENT> &array, T delta) {
  inline_add_to(array.ptr()(), delta, SIZE);
}

/*
 * Subtract the source from the destination array.
 */

template <typename T, size_t ALIGNMENT>
static void subtract_from(T *destination, const T *source, size_t size) {
  inline_subtract_from(assume_aligned<ALIGNMENT>(destination),
                       assume_aligned<ALIGNMENT>(source), size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void subtract_from(AlignedArray<T, SIZE, ALIGNMENT> &destination,
                          const AlignedArray<T, SIZE, ALIGNMENT> &source) {
  inline_subtract_from(destination.ptr()(), source.ptr()(), SIZE);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void operator-=(AlignedArray<T, SIZE, ALIGNMENT> &destination,
                       const AlignedArray<T, SIZE, ALIGNMENT> &source) {
  inline_subtract_from(destination.ptr()(), source.ptr()(), SIZE);
}

template <typename T, size_t ALIGNMENT, class C, class D>
static void subtract_from(AlignedData<T, ALIGNMENT, C> &destination,
                          const AlignedData<T, ALIGNMENT, D> &source) {
  inline_subtract_from(destination.ptr()(), source.ptr()(),
                       ::std::min(destination.size(), source.size()));
}

template <typename T, size_t ALIGNMENT, class C, class D>
static void subtract_from_frames_up(AlignedData<T, ALIGNMENT, C> &destination,
                                    const AlignedData<T, ALIGNMENT, D> &source,
                                    size_t framesUp) {
  inline_subtract_from(
      destination.frame(framesUp), source.ptr()(),
      ::std::min(
          ::std::max(
              0,
              static_cast<ssize_t>(destination.size()) -
                  framesUp *
                      AlignedData<T, ALIGNMENT, C>::Metric::alignmentElements),
          source.size()));
}

/*
 * Subtract the source from the destination array with a factor.
 */

template <typename T, size_t ALIGNMENT>
static void subtract_from_with_factor(T *destination, const T *source, T factor,
                                      size_t size) {
  inline_subtract_from_with_factor(assume_aligned<ALIGNMENT>(destination),
                                   assume_aligned<ALIGNMENT>(source), factor,
                                   size);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void
subtract_from_with_factor(AlignedArray<T, SIZE, ALIGNMENT> &destination,
                          const AlignedArray<T, SIZE, ALIGNMENT> &source,
                          T factor) {
  inline_subtract_from_with_factor(destination.ptr()(), source.ptr()(), factor,
                                   SIZE);
}

template <typename T, size_t ALIGNMENT, class C, class D>
static void
subtract_from_with_factor(AlignedData<T, ALIGNMENT, C> &destination,
                          const AlignedData<T, ALIGNMENT, D> &source,
                          T factor) {
  inline_subtract_from_with_factor(
      destination.ptr()(), source.ptr()(),
      ::std::min(destination.size(), factor, source.size()));
}

template <typename T, size_t ALIGNMENT, class C, class D>
static void
subtract_from_with_factor_frames_up(AlignedData<T, ALIGNMENT, C> &destination,
                                    const AlignedData<T, ALIGNMENT, D> &source,
                                    T factor, size_t framesUp) {
  inline_subtract_from_with_factor(
      destination.frame(framesUp), source.ptr()(), factor,
      ::std::min(
          ::std::max(
              0,
              static_cast<ssize_t>(destination.size()) -
                  framesUp *
                      AlignedData<T, ALIGNMENT, C>::Metric::alignmentElements),
          source.size()));
}

/*
 * Subtract a value from each element of the array.
 */

template <typename T, size_t ALIGNMENT = 0>
static void subtract_from(T *array, T delta, size_t size) {
  inline_subtract_from(assume_aligned<ALIGNMENT>(array), delta, size);
}

template <typename T, size_t ALIGNMENT, class C>
static void subtract_from(AlignedData<T, ALIGNMENT, C> &array, T delta) {
  inline_subtract_from(array.ptr()(), delta, array.size());
}

template <typename T, size_t ALIGNMENT, class C>
static void operator-=(AlignedData<T, ALIGNMENT, C> &array, T delta) {
  inline_subtract_from(array.ptr()(), delta, array.size());
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void subtract_from(AlignedArray<T, SIZE, ALIGNMENT> &array, T delta) {
  inline_subtract_from(array.ptr()(), delta, SIZE);
}

template <typename T, size_t SIZE, size_t ALIGNMENT>
static void operator-=(AlignedArray<T, SIZE, ALIGNMENT> &array, T delta) {
  inline_subtract_from(array.ptr()(), delta, SIZE);
}

} // namespace simpledsp::arrayops

#endif // SIMPLE_DSP_ARRAYOPS_H
