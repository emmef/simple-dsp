#ifndef SIMPLE_DSP_SIZE_H
#define SIMPLE_DSP_SIZE_H
/*
 * simple-dsp/util/size.h
 *
 * Copyright (C) 2015-2019 Michel Fleur.
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

#include <simple-dsp/algorithm/ranges.h>

namespace simpledsp::algorithm {

        template<typename T>
        using SizeLimits = simpledsp::algorithm::detail::LimitBase<T, algorithm::detail::LimitBaseType::SIZE>;

        template<typename T>
        using OffsetLimits = simpledsp::algorithm::detail::LimitBase<T, algorithm::detail::LimitBaseType::OFFSET>;

} // namespace simpledsp::algorithm



#endif //SIMPLE_DSP_SIZE_H
