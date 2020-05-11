#ifndef SIMPLE_DSP_CORE_INDEX_H
#define SIMPLE_DSP_CORE_INDEX_H
/*
 * simple-dsp/core/index.h
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
#include "bounds.h"
#include <algorithm>
#include <simple-dsp/core/attributes.h>
#include <stdexcept>

namespace simpledsp {

struct Index {

  template <typename S> sdsp_nodiscard static S checked(S index, S size) {
    if (index < size) {
      return index;
    }
    throw std::invalid_argument("IndexPolicy::index out of range");
  }

  template <typename S> sdsp_nodiscard static S wrapped(S index, S size) {
    return index % size;
  }

  template <typename S> sdsp_nodiscard static S unchecked(S index, S) {
    return index;
  }

  /**
   * Returns checked index, or unchecked if
   * SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
   */
  template <typename S> sdsp_nodiscard static S safe(S index, S size) {
#ifndef SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED
    return checked(index, size);
#else
    return unchecked(index, size);
#endif
  }

  /**
   * Returns unchecked index, or checked if
   * SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
   */
  template <typename S> sdsp_nodiscard static S unsafe(S index, S size) {
#ifndef SDSP_INDEX_POLICY_FORCE_UNSAFE_CHECKED
    return unchecked(index, size);
#else
    return checked(index, size);
#endif
  }

  struct Inclusive {
    template <typename S> sdsp_nodiscard static S checked(S index, S size) {
      if (index <= size) {
        return index;
      }
      throw std::invalid_argument("IndexPolicy::offset out of range");
    }

    template <typename S> sdsp_nodiscard static S wrapped(S index, S size) {
      return index % size;
    }

    template <typename S> sdsp_nodiscard static S unchecked(S index, S) {
      return index;
    }

    /**
     * Returns checked index, or unchecked if
     * SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
     */
    template <typename S> sdsp_nodiscard static S safe(S index, S size) {
#ifndef SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED
      return checked(index, size);
#else
      return unchecked(index, size);
#endif
    }

    /**
     * Returns unchecked index, or checked if
     * SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
     */
    template <typename S> sdsp_nodiscard static S unsafe(S index, S size) {
#ifndef SDSP_INDEX_POLICY_FORCE_UNSAFE_CHECKED
      return unchecked(index, size);
#else
      return checked(index, size);
#endif
    }
  };
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_INDEX_H
