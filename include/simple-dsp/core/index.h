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
#include <simple-dsp/core/bounds.h>
#include <simple-dsp/core/size.h>
#include <stdexcept>

namespace simpledsp {

namespace internal {
enum class IndexPolicyType { THROW, WRAP, UNCHECKED };

template <typename SizeType, IndexPolicyType type> struct IndexPolicyBase;

template <typename SizeType>
struct IndexPolicyBase<SizeType, IndexPolicyType::THROW> {
  sdsp_nodiscard static SizeType index(SizeType i, SizeType size) {
    if (i < size) {
      return i;
    }
    throw std::invalid_argument("IndexPolicy::index out of range");
  }
  sdsp_nodiscard static SizeType index_incl(SizeType o,
                                            SizeType inclusive_max) {
    if (o <= inclusive_max) {
      return o;
    }
    throw std::invalid_argument("IndexPolicy::offset out of range");
  }
};

template <typename SizeType>
struct IndexPolicyBase<SizeType, IndexPolicyType::WRAP> {
  sdsp_nodiscard sdsp_force_inline static SizeType
  index(SizeType i, SizeType size) noexcept {
    return i % size;
  }
  sdsp_nodiscard sdsp_force_inline static SizeType
  index_incl(SizeType o, SizeType inclusive_max) noexcept {
    return index(o, inclusive_max);
  }
};

template <typename SizeType>
struct IndexPolicyBase<SizeType, IndexPolicyType::UNCHECKED> {
  sdsp_nodiscard sdsp_force_inline static SizeType index(SizeType i,
                                                         SizeType) noexcept {
    return i;
  }
  sdsp_nodiscard sdsp_force_inline static SizeType
  index_incl(SizeType o, SizeType) noexcept {
    return o;
  }
};

} // namespace internal

struct Index {
  /**
   * Checked index policy that throws an exception on invalid indexes.
   */
  template <typename S = size_t>
  using Checked =
      internal::IndexPolicyBase<S, internal::IndexPolicyType::THROW>;

  /**
   * Index policy that wraps arguments within boundaries.
   */
  template <typename S = size_t>
  using Wrapped = internal::IndexPolicyBase<S, internal::IndexPolicyType::WRAP>;

  /**
   * Index policy that does not check anything at all.
   */
  template <typename S = size_t>
  using Unchecked =
      internal::IndexPolicyBase<S, internal::IndexPolicyType::UNCHECKED>;

  /**
   * Checked Index policy that throws an exception on invalid indexes, but
   * checking can be disabled by defining
   * SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED.
   */
  template <typename S = size_t>
  using Safe =
#ifndef SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED
      internal::IndexPolicyBase<S, internal::IndexPolicyType::THROW>;
#else
      IndexPolicyBase<S, IndexPolicyType::UNCHECKED>;
#endif

  /**
   * Index policy that does not check anything at all, but behavior can be
   * changed so that it throws on invalid indexes by defining
   * SDSP_INDEX_POLICY_FORCE_UNSAFE_CHECKED.
   */
  template <typename S = size_t>
  using Unsafe =
#ifndef SDSP_INDEX_POLICY_FORCE_UNSAFE_CHECKED
      internal::IndexPolicyBase<S, internal::IndexPolicyType::UNCHECKED>;
#else
      IndexPolicyBase<S, IndexPolicyType::THROW>;
#endif

  /**
   * Returns index if it is smaller than size and throws std::invalid_argument
   * otherwise.
   * @return index
   */
  template <typename S> sdsp_nodiscard static S checked(S index, S size) {
    return Checked<S>::index(index, size);
  }

  /**
   * Returns index, wrapped inside size and thus smaller than size.
   * @return wrapped index.
   */
  template <typename S> sdsp_nodiscard static S wrapped(S index, S size) {
    return Wrapped<S>::index(index, size);
  }

  /**
   * Returns index if it is smaller than size and throws std::invalid_argument
   * otherwise. Checkin can be disabled by defining
   * SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED.
   * @return index
   */
  template <typename S> sdsp_nodiscard static S safe(S index, S size) {
    return Safe<S>::index(index, size);
  }

  /**
   * Returns index without checking, but checking can be enabled by defining
   * SDSP_INDEX_POLICY_FORCE_UNSAFE_CHECKED.
   * @return index;
   */
  template <typename S> sdsp_nodiscard static S unsafe(S index, S size) {
    return Unsafe<S>::index(index, size);
  }

  /**
   * Returns index if it is equal to or smaller than size and throws
   * std::invalid_argument otherwise.
   * @return index
   */
  template <typename S> sdsp_nodiscard static S checked_incl(S index, S size) {
    return Checked<S>::index_incl(index, size);
  }

  /**
   * Returns index, wrapped inside size. Because the nature of wrapping, return
   * values are smaller than size by definition.
   * @return wrapped index.
   */
  template <typename S> sdsp_nodiscard static S wrapped_incl(S index, S size) {
    return Wrapped<S>::index_incl(index, size);
  }

  /**
   * Returns index if it is equal to or smaller than size and throws
   * std::invalid_argument otherwise. Checkin can be disabled by defining
   * SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED.
   * @return index
   */
  template <typename S> sdsp_nodiscard static S safe_incl(S index, S size) {
    return Safe<S>::index_incl(index, size);
  }

  /**
   * Returns index without checking, but checking can be enabled by defining
   * SDSP_INDEX_POLICY_FORCE_UNSAFE_CHECKED.
   * @return index;
   */
  template <typename S> sdsp_nodiscard static S unsafe_incl(S index, S size) {
    return Unsafe<S>::index_incl(index, size);
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_INDEX_H
