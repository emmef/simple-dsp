#ifndef SIMPLE_DSP_FRAME_H
#define SIMPLE_DSP_FRAME_H
/*
 * simple-dsp/util/frame.h
 *
 * Added by michel on 2020-05-12
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

#include <simple-dsp/core/alignment.h>
#include <simple-dsp/core/index.h>

namespace simpledsp {

template <typename T, size_t N, bool aligned = true> struct Frame {
  static_assert(N > 0, "Frame: must have at least one element.");
  friend Frame<T, N, !aligned>;

  static constexpr size_t size = N;
  using value_type = T;

  // Access

  sdsp_nodiscard T operator[](size_t i) const noexcept {
    return d[Index::unchecked(i, N)];
  }
  sdsp_nodiscard T &operator[](size_t i) noexcept {
    return d[Index::unchecked(i, N)];
  }
  sdsp_nodiscard T operator()(size_t i) const { return d[Index::safe(i, N)]; }
  sdsp_nodiscard T &operator()(size_t i) { return d[Index::safe(i, N)]; }

  void set(T v) noexcept {
    for (size_t i = 0; i < N; i++) {
      d[i] = v;
    }
  }
  /**
   * Adds a constant value to all elements.
   * @param f The constant value
   * @return this Frame.
   */
  void add(T v) noexcept {
    for (size_t i = 0; i < N; i++) {
      d[i] += v;
    }
  };

  /**
   * Calculates the dot-product of this frame with the other frame.
   * @param f The other frame
   * @return The dot product.
   */
  template<bool A>
  sdsp_nodiscard T dot(const Frame<T,N,A> &f) const noexcept {
    T sum = d[0] *= f[0];
    for (size_t i = 1; i < N; i++) {
      sum += d[i] *= f[i];
    }
    return sum;
  }

  /**
   * Calculates the self-product of this frame, which is equal to the square sum
   * of all elements.
   * @return
   */
  sdsp_nodiscard T self() const noexcept { return dot(*this); }

  /*
   * Addition
   */

  template<bool A>
  Frame &operator+=(const Frame<T,N,A> &f) noexcept {
    for (size_t i = 0; i < N; i++) {
      d[i] += f[i];
    }
    return *this;
  }

  template<bool A>
  friend Frame operator+(Frame f1, const Frame<T,N,A> &f2) noexcept {
    f1 += f2;
    return f1;
  }

  /*
   * Subtraction
   */

  template <bool A>
  Frame &operator-=(const Frame<T,N,A> &f) noexcept {
    for (size_t i = 0; i < N; i++) {
      d[i] -= f[i];
    }
    return *this;
  }

  template <bool A>
  friend Frame operator-(Frame f1, const Frame<T,N,A> &f2) noexcept {
    f1 -= f2;
    return f1;
  }

  /*
   * Multiplication
   */

  Frame &operator*=(T v) noexcept {
    for (size_t i = 0; i < N; i++) {
      d[i] *= v;
    }
    return *this;
  }

  friend Frame operator*(Frame f, T v) noexcept {
    f *= v;
    return f;
  }

  friend Frame operator*(T v, Frame f) noexcept {
    f *= v;
    return f;
  }

  /*
   * Division
   */

  Frame &operator/=(T v) noexcept {
    for (size_t i = 0; i < N; i++) {
      d[i] /= v;
    }
    return *this;
  }

  friend Frame operator/(Frame f, T v) noexcept {
    f /= v;
    return f;
  }

  /**
   * Assign from different align
   */
//  template<bool A>
  Frame &operator=(const Frame<T,N,!aligned> &other) {
    for (size_t i = 0; i < N; i++) {
      d[i] = other.d[i];
    }
    return *this;
  }

private:
  static constexpr AlignedFor<T> align;

  alignas(aligned ? align.bytes : alignof(T)) T d[N];
};




} // namespace simpledsp

#endif // SIMPLE_DSP_FRAME_H
