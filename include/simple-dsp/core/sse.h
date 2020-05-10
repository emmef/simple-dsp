#ifndef SIMPLE_DSP_CORE_SSE_H
#define SIMPLE_DSP_CORE_SSE_H
/*
 * simple-dsp/core/sse.h
 *
 * Added by michel on 2020-05-10
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

#if defined(__SSE__) &&                                                        \
    (defined(__amd64__) || defined(__x86_64__) || defined(__i386__))
#include <xmmintrin.h>
#define SDSP_SSE_INSTRUCTIONS_AVAILABLE 1
#else
#undef SSE_INSTRUCTIONS_AVAILABLE
#endif

namespace simpledsp {
struct SseState {

  enum class Rounding {
    kRoundNearest = 0,
    kRoundNegative,
    kRoundPositive,
    kRoundToZero,
  };

  /**
   * @return true if SSE state control is supported and false otherwise.
   */
  static bool is_supported() noexcept {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
    return true;
#else
    return false;
#endif
  }

  /**
   * @return the state of the SSE control register and zero if SSE state
   * control is not supported.
   */
  static unsigned int get() {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
    return _mm_getcsr();
#else
    return 0;
#endif
  }

  /**
   * Sets the specified rounding mode in the SSE control register or does
   * nothing if SSE state control is not supported.
   *
   * Adapted from code by mystran @ kvraudio.
   * http://www.kvraudio.com/forum/viewtopic.php?t=312228&postdays=0&postorder=asc&start=0
   */
  static void set_rounding_mode(Rounding mode) {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
    // bits: 15 = flush to zero | 6 = denormals are zero
    // bitwise-OR with exception masks 12:7 (exception flags 5:0)
    // rounding 14:13, 00 = nearest, 01 = neg, 10 = pos, 11 = to zero
    // The enum above is defined in the same order so just shift it up
    _mm_setcsr(0x8040 | 0x1f80 | ((unsigned int)mode << 13));
#endif
  }

  /**
   * Sets the specified SSE state control or does nothing if SSE state control
   * is not supported.
   */
  static void set(unsigned int state) {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
    // clear exception flags, just in case (probably pointless)
    _mm_setcsr(state & (~0x3f));
#endif
  }

  class Guard {
  private:
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
    unsigned int sse_control_store;
#endif
  public:
    Guard() {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
      sse_control_store = get();
#endif
    }

    ~Guard() {
#ifdef SDSP_SSE_INSTRUCTIONS_AVAILABLE
      set(sse_control_store);
#endif
    }
  };
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_SSE_H
