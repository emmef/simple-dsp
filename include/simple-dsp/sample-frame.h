#ifndef SIMPLE_DSP_SAMPLE_FRAME_H
#define SIMPLE_DSP_SAMPLE_FRAME_H
/*
 * simple-dsp/sample-frame.h
 *
 * Added by michel on 2019-11-03
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
#include <simple-dsp/core/addressing.h>

namespace simpledsp {

template <typename T, size_t N> class SampleFrame {
  T x_[N];

public:
  SampleFrame() : SampleFrame(T(0)) {}
  explicit SampleFrame(const T &value) { assignScalar(value); }

  SampleFrame(const SampleFrame &source) = default;
  SampleFrame(SampleFrame &&source) noexcept = default;
  SampleFrame &operator=(const SampleFrame &) = default;

  SampleFrame &operator=(const T &value) {
    assignScalar(value);
    return *this;
  }

  void assignScalar(const T &value) const {
    for (T *dst = &x_; dst < &x_ + this->N; ++dst) {
      *dst = value;
    }
  }

  void assignFrame(const SampleFrame &source) const { this->operator=(source); }

  T &operator[](size_t i) { return x_[Index::Array::index(i, N)]; }

  const T &operator[](size_t i) const { return x_[Index::Array::index(i, N)]; }

  T &ref(size_t i) { return x_[Index::Method::index(i, N)]; }

  const T &ref(size_t i) const { return x_[Index::Method::index(i, N)]; }

  SampleFrame &operator+=(const SampleFrame &value) {
    for (T *dst = &x_, *src = &value.x_; dst < &x_ + this->N; ++dst, ++src) {
      *dst += *src;
    }
    return *this;
  }

  SampleFrame &operator+=(SampleFrame &&value) {
    for (T *dst = &x_, *src = &value.x_; dst < &x_ + this->N; ++dst, ++src) {
      *dst += *src;
    }
    return *this;
  }

  SampleFrame &operator-=(const SampleFrame &value) {
    for (T *dst = &x_, *src = &value.x_; dst < &x_ + this->N; ++dst, ++src) {
      *dst -= *src;
    }
    return *this;
  }

  SampleFrame &operator-=(SampleFrame &&value) {
    for (T *dst = &x_, *src = &value.x_; dst < &x_ + this->N; ++dst, ++src) {
      *dst -= *src;
    }
    return *this;
  }

  SampleFrame &operator*=(const T &value) {
    for (T *dst = &x_; dst < &x_ + this->N; ++dst) {
      *dst *= value;
    }
    return *this;
  }

  SampleFrame &operator/=(const T &value) {
    for (T *dst = &x_; dst < &x_ + this->N; ++dst) {
      *dst /= value;
    }
    return *this;
  }

  T dot(const SampleFrame &value) {
    T result = 0;
    for (T *dst = &x_, *src = &value.x_; dst < &x_ + this->N; ++dst, ++src) {
      result += *dst * *src;
    }
    return result;
  }

  T sqr() {
    T result = 0;
    for (T *dst = &x_; dst < &x_ + this->N; ++dst) {
      result += *dst * *dst;
    }
    return result;
  }
};

template <typename T, size_t N>
SampleFrame<T, N> operator+(const SampleFrame<T, N> &v1,
                            const SampleFrame<T, N> &v2) {
  SampleFrame result(v1);
  result += v2;
  return result;
}

template <typename T, size_t N>
SampleFrame<T, N> operator+(SampleFrame<T, N> &&v1,
                            const SampleFrame<T, N> &v2) {
  v1 += v2;
  return v1;
}

template <typename T, size_t N>
SampleFrame<T, N> operator+(const SampleFrame<T, N> &v1,
                            SampleFrame<T, N> &&v2) {
  v2 += v1;
  return v2;
}

template <typename T, size_t N>
SampleFrame<T, N> operator+(SampleFrame<T, N> &&v1, SampleFrame<T, N> &&v2) {
  v1 += v2;
  return v1;
}

template <typename T, size_t N>
SampleFrame<T, N> operator-(const SampleFrame<T, N> &v1,
                            const SampleFrame<T, N> &v2) {
  SampleFrame result(v1);
  result -= v2;
  return result;
}

template <typename T, size_t N>
SampleFrame<T, N> operator-(SampleFrame<T, N> &&v1,
                            const SampleFrame<T, N> &v2) {
  v1 -= v2;
  return v1;
}

template <typename T, size_t N>
SampleFrame<T, N> operator-(const SampleFrame<T, N> &v1,
                            SampleFrame<T, N> &&v2) {
  v2 -= v1;
  return v2;
}

template <typename T, size_t N>
SampleFrame<T, N> operator-(SampleFrame<T, N> &&v1, SampleFrame<T, N> &&v2) {
  v1 -= v2;
  return v1;
}

template <typename T, size_t N>
SampleFrame<T, N> operator*(const SampleFrame<T, N> &v1, const T &v2) {
  SampleFrame result(v1);
  result *= v2;
  return result;
}

template <typename T, size_t N>
SampleFrame<T, N> operator*(SampleFrame<T, N> &&v1, const T &v2) {
  v1 *= v2;
  return v1;
}

template <typename T, size_t N>
SampleFrame<T, N> operator*(const T &v1, const SampleFrame<T, N> &v2) {
  SampleFrame result(v2);
  result *= v1;
  return result;
}

template <typename T, size_t N>
SampleFrame<T, N> operator*(const T &v1, SampleFrame<T, N> &&v2) {
  v2 *= v1;
  return v1;
}

template <typename T, size_t N>
SampleFrame<T, N> operator/(const SampleFrame<T, N> &v1, const T &v2) {
  SampleFrame result(v1);
  result /= v2;
  return result;
}

template <typename T, size_t N>
SampleFrame<T, N> operator/(SampleFrame<T, N> &&v1, const T &v2) {
  v1 /= v2;
  return v1;
}

} // namespace simpledsp

#endif // SIMPLE_DSP_SAMPLE_FRAME_H
