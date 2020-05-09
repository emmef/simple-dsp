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

enum class IndexPolicyType { THROW, WRAP, UNCHECKED };

namespace internal {
template <typename SizeType, IndexPolicyType type> struct IndexPolicyBase;

template <typename SizeType>
struct IndexPolicyBase<SizeType, IndexPolicyType::THROW> {
  sdsp_nodiscard static SizeType index(SizeType i, SizeType size) {
    if (i < size) {
      return i;
    }
    throw std::invalid_argument("IndexPolicy::index out of range");
  }
  sdsp_nodiscard static SizeType index_incl(SizeType o, SizeType maxOffset) {
    if (o <= maxOffset) {
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
  index_incl(SizeType o, SizeType maxOffset) noexcept {
    return o % (maxOffset + 1);
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
  template <typename S = size_t>
  using Safe =
#ifndef SDSP_INDEX_POLICY_FORCE_SAFE_UNCHECKED
      internal::IndexPolicyBase<S, IndexPolicyType::THROW>;
#else
      IndexPolicyBase<S, IndexPolicyType::UNCHECKED>;
#endif

  template <typename S = size_t>
  using Unsafe =
#ifndef SDSP_INDEX_POLICY_FORCE_UNSAFE_CHECKED
      internal::IndexPolicyBase<S, IndexPolicyType::UNCHECKED>;
#else
      IndexPolicyBase<S, IndexPolicyType::THROW>;
#endif

  template <typename S = size_t>
  using Throw = internal::IndexPolicyBase<S, IndexPolicyType::THROW>;

  template <typename S = size_t>
  using Wrap = internal::IndexPolicyBase<S, IndexPolicyType::WRAP>;

  template <typename S = size_t>
  using Unchecked = internal::IndexPolicyBase<S, IndexPolicyType::UNCHECKED>;

  template <typename S> sdsp_nodiscard static S checked(S i, S size) {
    return Throw<S>::index(i, size);
  }

  template <typename S> sdsp_nodiscard static S safe(S i, S size) {
    return Safe<S>::index(i, size);
  }

  template <typename S> sdsp_nodiscard static S unsafe(S i, S size) {
    return Unsafe<S>::index(i, size);
  }

  template <typename S> sdsp_nodiscard static S checked_incl(S i, S size) {
    return Throw<S>::index_incl(i, size);
  }

  template <typename S> sdsp_nodiscard static S safe_incl(S i, S size) {
    return Safe<S>::index_incl(i, size);
  }

  template <typename S> sdsp_nodiscard static S unsafe_incl(S i, S size) {
    return Unsafe<S>::index_incl(i, size);
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_INDEX_H
