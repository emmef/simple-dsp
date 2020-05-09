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

#if defined(__SSE__) &&                                                        \
    (defined(__amd64__) || defined(__x86_64__) || defined(__i386__))
#include <xmmintrin.h>
#define SDSP_SSE_INSTRUCTIONS_AVAILABLE 1
#else
#undef SSE_INSTRUCTIONS_AVAILABLE
#endif

namespace simpledsp {
// RAII FPU state class, sets FTZ and DAZ and rounding, no exceptions
// Adapted from code by mystran @ kvraudio
// http://www.kvraudio.com/forum/viewtopic.php?t=312228&postdays=0&postorder=asc&start=0

namespace sse {

enum class Rounding {
  kRoundNearest = 0,
  kRoundNegative,
  kRoundPositive,
  kRoundToZero,
};

static bool supports_sse_state() noexcept {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
  return true;
#else
  return false;
#endif
}

static unsigned int get_sse_state() {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
  return _mm_getcsr();
#else
  return 0;
#endif
}

static void set_sse_rounding_mode(Rounding mode) {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
  // bits: 15 = flush to zero | 6 = denormals are zero
  // bitwise-OR with exception masks 12:7 (exception flags 5:0)
  // rounding 14:13, 00 = nearest, 01 = neg, 10 = pos, 11 = to zero
  // The enum above is defined in the same order so just shift it up
  _mm_setcsr(0x8040 | 0x1f80 | ((unsigned int)mode << 13));
#endif
}

static void set_sse_state(unsigned int state) {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
  // clear exception flags, just in case (probably pointless)
  _mm_setcsr(state & (~0x3f));
#endif
}

class ZFPUState {
private:
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
  unsigned int sse_control_store;
#endif
public:
  ZFPUState(Rounding mode = Rounding::kRoundToZero) {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
    sse_control_store = get_sse_state();
    set_sse_rounding_mode(mode);
#endif
  }

  ~ZFPUState() {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
    set_sse_state(sse_control_store);
#endif
  }
};

} // namespace sse

namespace internal {
namespace {
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

  static inline FPTYPE flush(FPTYPE &value) { return value; }

  static constexpr bool normalizes = false;

  static constexpr size_t bits = 8 * sizeof(FPTYPE);

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

  static constexpr size_t bits = 8 * sizeof(FPTYPE);

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

  static constexpr bool normalizes = false;

  static constexpr size_t bits = 8 * sizeof(FPTYPE);

  static const char *method() { return "IEEE-559 64-bit double precision"; }
};

} // anonymous namespace
} // namespace internal

template <typename FPTYPE>
class Denormal {
public:
  using Base = internal::Normalize<
      FPTYPE, !std::numeric_limits<FPTYPE>::is_iec559 ? 0 : sizeof(FPTYPE)>;

  using Base::flush;
  using Base::get_flushed;
  using Base::method;
  static inline FPTYPE flush_and_get(FPTYPE &value) {
    flush(value);
    return value;
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_DENORMAL_H
