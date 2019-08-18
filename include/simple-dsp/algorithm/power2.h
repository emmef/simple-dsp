#ifndef SIMPLE_DSP_POWER2_H
#define SIMPLE_DSP_POWER2_H
/*
 * simple-dsp/power2.h
 *
 * Added by michel on 2019-08-18
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
#include <cstddef>

namespace simpledsp::algorithm::detail {

    template<typename SIZE_T, size_t SIZE_OF_SIZE_T, bool constExpr>
    struct FillBitsToRight
    {
    };

    template<class SIZE_T>
    struct FillBitsToRight<SIZE_T, 1, false>
    {
        static inline SIZE_T fill(const SIZE_T x)
        {
            SIZE_T n = x;
            n = n | (n >> 1);
            n = n | (n >> 2);
            n = n | (n >> 4);
            return n;
        }
    };

    template<typename SIZE_T>
    struct FillBitsToRight<SIZE_T, 2, false>
    {
        static inline SIZE_T fill(const SIZE_T x)
        {
            SIZE_T n = x;
            n = n | (n >> 1);
            n = n | (n >> 2);
            n = n | (n >> 4);
            n = n | (n >> 8);
            return n;
        }
    };

    template<typename SIZE_T>
    struct FillBitsToRight<SIZE_T, 4, false>
    {
        static inline SIZE_T fill(const SIZE_T x)
        {
            SIZE_T n = x;
            n = n | (n >> 1);
            n = n | (n >> 2);
            n = n | (n >> 4);
            n = n | (n >> 8);
            n = n | (n >> 16);
            return n;
        }
    };

    template<typename SIZE_T>
    struct FillBitsToRight<SIZE_T, 8, false>
    {
        static inline SIZE_T fill(const SIZE_T x)
        {
            SIZE_T n = x;
            n = n | (n >> 1);
            n = n | (n >> 2);
            n = n | (n >> 4);
            n = n | (n >> 8);
            n = n | (n >> 16);
            n = n | (n >> 32);
            return n;
        }
    };

    template<typename SIZE_T>
    struct FillBitsToRight<SIZE_T, 16, false>
    {
        static inline SIZE_T fill(const SIZE_T x)
        {
            SIZE_T n = x;
            n = n | (n >> 1);
            n = n | (n >> 2);
            n = n | (n >> 4);
            n = n | (n >> 8);
            n = n | (n >> 16);
            n = n | (n >> 32);
            n = n | (n >> 64);
            return n;
        }
    };

    template<typename SIZE_T>
    class FillBitsToRight<SIZE_T, 0, true>
    {
        template<size_t N>
        static constexpr SIZE_T fillN(const SIZE_T n)
        {
            return N < 2 ? n : fillN<N / 2>(n) | (fillN<N / 2>(n) >> (N / 2));
        };
    public:
        static constexpr SIZE_T fill(const SIZE_T n)
        {
            return fillN<8 * sizeof(SIZE_T)>(n);
        };
    };

    template<bool constExpr, typename SIZE_T = size_t>
    class PowerOfTwoHelper : public FillBitsToRight<SIZE_T, constExpr ? 0 : sizeof(SIZE_T), constExpr>
    {
        using FillBitsToRight<SIZE_T, constExpr ? 0 : sizeof(SIZE_T), constExpr>::fill;

        static constexpr SIZE_T getAligned(SIZE_T value, SIZE_T alignment)
        {
            return (value + (alignment - 1)) & (~(alignment - 1));
        }

    public:

        static constexpr bool isMinusOne(const SIZE_T value)
        {
            return value != 0 && fill(value) == value;
        }

        static constexpr bool is(const SIZE_T value)
        {
            return value >= 2 && isMinusOne(value - 1);
        }

        /**
         * Returns value if it is a power of two or else the next power of two that is greater.
         */
        static constexpr SIZE_T nextOrSame(const SIZE_T value)
        {
            return fill(value - 1) + 1;
        }

        /**
         * Returns value if it is a power of two or else the next power of two that is smaller.
         */
        static constexpr SIZE_T previousOrSame(const SIZE_T value)
        {
            return value < 1 ? value : nextOrSame(value / 2 + 1);
        }

        /**
         * Returns the value if it is aligned to power_of_two, the first higher
         * value that is aligned to power_of_two or zero if the provided power of two
         * is not actually a power of two.
         *
         * @param value Value to be aligned
         * @param power_of_two The power of two to align to
         * @return the aligned value
         */
        static constexpr SIZE_T alignedWith(const SIZE_T value, const SIZE_T power_of_two)
        {
            return getAligned(value, power_of_two);
        }
    };
} // namespace simpledsp algorithm details

namespace simpledsp::algorithm {
    struct Power2 : public detail::PowerOfTwoHelper<false>
    {
        using constant = detail::PowerOfTwoHelper<true>;
    };

}


#endif //SIMPLE_DSP_POWER2_H
