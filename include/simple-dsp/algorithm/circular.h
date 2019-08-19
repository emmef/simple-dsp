#ifndef SIMPLE_DSP_CIRCULAR_H
#define SIMPLE_DSP_CIRCULAR_H
/*
 * simple-dsp/algorithm/circular.h
 *
 * Added by michel on 2019-08-19
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

#include <type_traits>
#include <limits>
#include <simple-dsp/algorithm/power2.h>
#include <simple-dsp/attributes.h>

namespace simpledsp::algorithm {

    template <typename SIZE_T>
    struct CircularArithmicBase {
        static_assert(
                std::is_integral<SIZE_T>::value && !std::is_signed<SIZE_T>::value,
                "SIZE_T must be an integral, unsigned type");

        static constexpr SIZE_T maximumMask = std::numeric_limits<SIZE_T>::max() >> 1;

        sdsp_nodiscard static constexpr SIZE_T properCircularSize(
                SIZE_T requestedSize) {

            return requestedSize > static_cast<SIZE_T>(2)
                   ? Power2::sameOrBigger(requestedSize)
                   : static_cast<SIZE_T>(2);
        }

        sdsp_nodiscard static constexpr SIZE_T properCircularMask(
                SIZE_T requestedSize) {

            return Power2::surroundingMask(requestedSize) & maximumMask;
        }

        sdsp_nodiscard static constexpr SIZE_T next(
                SIZE_T circularPointer, SIZE_T uncheckedMask) {

            return (circularPointer + 1) & uncheckedMask;
        }

        sdsp_nodiscard static constexpr SIZE_T previous(
                SIZE_T circularPointer, SIZE_T uncheckedMask) {

            return (circularPointer - 1) & uncheckedMask;
        }

        sdsp_nodiscard static constexpr SIZE_T size(
                SIZE_T uncheckedMask) {

            return uncheckedMask + 1;
        }

        sdsp_nodiscard static constexpr SIZE_T add(
                SIZE_T circularPointer, SIZE_T delta, SIZE_T uncheckedMask) {

            return (circularPointer + delta) & uncheckedMask;
        }

        sdsp_nodiscard static constexpr SIZE_T subtract(
                SIZE_T circularPointer, SIZE_T delta, SIZE_T uncheckedMask) {

            return (circularPointer + uncheckedMask + 1 - delta & uncheckedMask) & uncheckedMask;
        }

        sdsp_nodiscard static constexpr SIZE_T subtractUnsafe(
                SIZE_T circularPointer, SIZE_T delta, SIZE_T uncheckedMask) {

            return (delta & circularPointer + uncheckedMask + 1 - delta & uncheckedMask) & uncheckedMask;
        }

        static constexpr void setNext(
                SIZE_T &circularPointer, SIZE_T uncheckedMask) {

            ++circularPointer &= uncheckedMask;
        }

        static constexpr void setPrevious(
                SIZE_T &circularPointer, SIZE_T uncheckedMask) {

            --circularPointer &= uncheckedMask;
        }

    };

    template <typename SIZE_T>
    class CircularMetricBase {
        SIZE_T mask;
        using Arithmic = CircularArithmicBase<SIZE_T>;
    public:
        explicit CircularMetricBase(SIZE_T requestedSize)
        : mask(Arithmic::properCircularMask(requestedSize)) { }

        sdsp_nodiscard SIZE_T getSize() const { return mask + 1;}

        sdsp_nodiscard SIZE_T getMask() const { return mask;}

        sdsp_nodiscard SIZE_T next(SIZE_T pointer) const { return Arithmic::next(pointer, mask); }

        sdsp_nodiscard SIZE_T previous(SIZE_T pointer) const { return Arithmic::previous(pointer, mask); }

        void setNext(SIZE_T pointer) const { Arithmic::setNext(pointer, mask); }

        void setPrevious(SIZE_T pointer) const { Arithmic::setPrevious(pointer, mask); }

        sdsp_nodiscard SIZE_T add(SIZE_T pointer, SIZE_T delta) const { return Arithmic::add(pointer, delta, mask); }

        sdsp_nodiscard SIZE_T subtract(SIZE_T pointer, SIZE_T delta) const { return Arithmic::subtract(pointer, delta, mask); }

        sdsp_nodiscard bool setSizeCheck(size_t requestedSize) {
            SIZE_T newMask = Arithmic::properCircularMask(requestedSize);
            if (newMask + 1 >= requestedSize) {
                mask = newMask;
                return true;
            }
            return false;
        }

        void setSize(size_t requestedSize) {
            mask = Arithmic::properCircularMask(requestedSize);
        }
    };

    using CircularArithmic = CircularArithmicBase<size_t>;
    using CircularMetric = CircularArithmicBase<size_t>;

} // namespace simpledsp::algorithm

#endif //SIMPLE_DSP_CIRCULAR_H
