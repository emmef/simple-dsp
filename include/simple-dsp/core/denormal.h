#ifndef SIMPLE_DSP_CORE_DENORMAL_H
#define SIMPLE_DSP_CORE_DENORMAL_H
/*
 * simple-dsp/core/denormal.h
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

#include <cinttypes>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace simpledsp {

namespace internal {

template <typename FPTYPE, size_t selector> struct Normalize {
  static_assert(std::is_floating_point<FPTYPE>::value,
                "FPTYPE must be a floating-point type");

  static inline FPTYPE get_flushed(FPTYPE value) { return value; }

  static inline void flush(FPTYPE &) {}

  static constexpr bool normalizes = false;

  static constexpr size_t bits = 8 * sizeof(FPTYPE);

  static const char *method() {
    return "None: IEEE-559 compliant, but denormal definition for this size "
           "not known";
  }
};

/**
 * Specialization for non ieee-559/754 floating point formats.
 */
template <typename FPTYPE> struct Normalize<FPTYPE, 0> {
  static inline FPTYPE get_flushed(FPTYPE value) { return value; }

  static inline void flush(FPTYPE &value) { return value; }

  static constexpr bool normalizes = false;

  static const char *method() { return "None: Not IEEE-559 compliant"; }
};

/**
 * Specialization for 32-bit single-precision floating
 */
template <typename FPTYPE> struct Normalize<FPTYPE, 4> {
  static inline FPTYPE get_flushed(FPTYPE value) {
    union {
      FPTYPE f;
      int32_t i;
    } v;

    v.f = value;

    return v.i & 0x7f800000 ? value : 0.0f;
  }

  static inline void flush(FPTYPE &value) {
    union {
      FPTYPE f;
      int32_t i;
    } v;

    v.f = value;
    if (v.i & 0x7f800000) {
      return;
    }
    value = 0.0;
  }

  static constexpr bool normalizes = true;

  static const char *method() { return "IEEE-559 32-bit single precision"; }
};

template <typename FPTYPE> struct Normalize<FPTYPE, 8> {
  static inline FPTYPE get_flushed(FPTYPE value) {
    union {
      FPTYPE f;
      int64_t i;
    } v;

    v.f = value;

    return v.i & 0x7ff0000000000000L ? value : 0;
  }

  static inline void flush(FPTYPE &value) {
    union {
      FPTYPE f;
      int64_t i;
    } v;

    v.f = value;

    if (v.i & 0x7ff0000000000000L) {
      return;
    }
    value = 0.0;
  }

  static constexpr size_t bits = 8 * sizeof(FPTYPE);

  static const char *method() { return "IEEE-559 64-bit double precision"; }
};

} // namespace internal

class Normalization {
  template <typename T>
  using Base =
      internal::Normalize<T,
                          !std::numeric_limits<T>::is_iec559 ? 0 : sizeof(T)>;

public:
  /**
   * Returns whether normalization for the floating point type is supported.
   * Support is purely algorithmic and says nothing about performance impact of
   * denormal numbers on the particular processor architecture.
   */
  template <typename FPTYPE> static constexpr bool supported() noexcept {
    return Base<FPTYPE>::normalizes;
  }

  /**
   * Flushes value to zero if it is denormal and the floating point type is
   * iec559 compliant and does nothing otherwise.
   */
  template <typename FPTYPE> static inline void flush(FPTYPE &value) noexcept {
    Base<FPTYPE>::flush(value);
  }

  /**
   * Returns value or zero if it is denormal and the floating point type is
   * iec559 compliant.
   */
  template <typename FPTYPE>
  static inline FPTYPE get_flushed(FPTYPE value) noexcept {
    return Base<FPTYPE>::get_flushed(value);
  }

  /**
   * Returns value, where value if set to zero if it is denormal and the
   * floating point type is iec559 compliant.
   */
  template <typename FPTYPE> static inline FPTYPE flush_and_get(FPTYPE &value) {
    flush(value);
    return value;
  }

  /**
   * Returns a brief description of the type of floatingpoint type with regards
   * to denormal representations.
   */
  template <typename FPTYPE>
  static inline const char *denormalization_type() noexcept {
    return Base<FPTYPE>::method();
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_DENORMAL_H
