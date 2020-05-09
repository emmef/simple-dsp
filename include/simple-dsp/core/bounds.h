#ifndef SIMPLE_DSP_CORE_BOUNDS_H
#define SIMPLE_DSP_CORE_BOUNDS_H

/*
 * simple-dsp/core/bounds.h
 *
 * Added by michel on 2019-09-12
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

#include <algorithm>
#include <cstddef>
#include <simple-dsp/core/attributes.h>
#include <simple-dsp/core/bits.h>
#include <type_traits>

namespace simpledsp {

template <typename T>
sdsp_nodiscard static constexpr bool is_within(const T value, const T minimum,
                                               const T maximum) {
  return value == ::std::clamp(value, minimum, maximum);
}

template <typename T>
static constexpr bool is_within_excl(const T value, const T minimum,
                                     const T maximum) {
  return value > minimum && value < maximum;
}

} // namespace simpledsp

#endif // SIMPLE_DSP_CORE_BOUNDS_H
