#ifndef SIMPLE_DSP_RANGES_H
#define SIMPLE_DSP_RANGES_H
/*
 * simple-dsp/util/algorithm.h
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
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace simpledsp::algorithm {

    template<typename T>
    static constexpr T min(T v1, T v2) {
        return v1 < v2 ? v1 : v2;
    }

    template<typename T, typename V>
    static constexpr T max(T v1, T v2) {
        return v1 > v2 ? v1 : v2;
    }

    template<typename T>
    static constexpr bool is_within(const T value, const T minimum, const T maximum) {
        return value >= minimum && value <= maximum;
    }

    template<typename T>
    static constexpr bool is_within_excl(const T value, const T minimum, const T maximum) {
        return value > minimum && value < maximum;
    }

    template<typename T>
    static constexpr T clamp(const T value, const T minimum, const T maximum) {
        return value < minimum ? minimum : value > maximum ? maximum : value;
    }

    namespace detail {

        template<typename T, int UNSIGNED>
        struct RangeBase {
        };

        template<typename T>
        struct RangeBase<T, 0> {

            static constexpr bool isWithin(const T value, const T minimum, const T maximum) {
                return simpledsp::algorithm::is_within<T>(value, minimum, maximum);
            }

            static constexpr bool isExclusiveWithin(const T value, const T minimum, const T maximum) {
                return simpledsp::algorithm::is_within_excl(value, minimum, maximum);
            }

            static constexpr T clamp(const T value, const T minimum, const T maximum) {
                return simpledsp::algorithm::clamp(value, minimum, maximum);
            }
        };

        static const char *const RANGE_BASE_PRODUCT_NOT_IN_RANGE =
                "RangeBase::product of values not in range";

        static const char *const RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE =
                "RangeBase::values and product of values not in range";

        static const char *const RANGE_BASE_VALUE_NOT_IN_RANGE =
                "RangeBase::value not in range";

        static const char *const RANGE_BASE_SUM_NOT_IN_RANGE =
                "RangeBase::sum of values not in range";

        template<typename T>
        struct RangeBase<T, 1> {

            static_assert(
                    std::is_integral<T>::value,
                    "Type T must be integral");
            static_assert(
                    !std::is_signed<T>::value,
                    "Type T must be unsigned");

            static constexpr bool isWithin(const T value, const T minimum, const T maximum) {
                return simpledsp::algorithm::is_within<T>(value, minimum, maximum);
            }

            static constexpr bool isExclusiveWithin(const T value, const T minimum, const T maximum) {
                return simpledsp::algorithm::is_within_excl(value, minimum, maximum);
            }

            static constexpr T clamp(const T value, const T minimum, const T maximum) {
                return simpledsp::algorithm::clamp(value, minimum, maximum);
            }

            static constexpr bool isSumWithin(const T v1, const T v2, const T min, const T max) {
                return v1 >= min
                       ? v1 <= max && max - v1 >= v2
                       : v2 <= max && max - v2 >= v1 && v2 - min >= min - v1;
            }

            static constexpr bool areSumAndValuesWithin(const T v1, const T v2, const T min, const T max) {
                return v1 >= min && v2 >= min && v1 <= max && max - v1 >= v2;
            }

            static constexpr bool isProductWithin(const T v1, const T v2, const T min, const T max) {
                return (v1 == 0 || v2 == 0) ? min == 0 : v1 <= max && max / v1 >= v2 && v1 * v2 >= min;
            }

            static constexpr bool areProductAndValuesWithin(const T v1, const T v2, const T min, const T max) {
                return v1 >= min && v2 >= min && v1 <= max && v2 <= max && (v1 > 0 ? max / v1 >= v2 : min == 0);
            }

            static constexpr size_t validValue(
                    const T value, const T min, const T max,
                    const char *msg = RANGE_BASE_VALUE_NOT_IN_RANGE) {

                if (isWithin(value, min, max)) {
                    return value;
                }
                throw std::invalid_argument(msg);
            }

            static constexpr size_t validProduct(
                    const T v1, const T v2, const T min, const T max,
                    const char *msg = RANGE_BASE_PRODUCT_NOT_IN_RANGE) {

                if (isProductWithin(v1, v2, min, max)) {
                    return v1 * v2;
                }

                throw std::invalid_argument(msg);
            }

            static constexpr size_t validProductAndValues(
                    const T v1, const T v2, const T min, const T max,
                    const char *msg = RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE) {

                if (areProductAndValuesWithin(v1, v2, min, max)) {
                    return v1 * v2;
                }
                throw std::invalid_argument(msg);
            }

            static constexpr size_t validProductGetFirstValue(
                    const T v1, const T v2, const T min, const T max,
                    const char *msg = RANGE_BASE_PRODUCT_NOT_IN_RANGE) {

                if (isProductWithin(v1, v2, min, max)) {
                    return v1;
                }
                throw std::invalid_argument(msg);
            }

            static constexpr size_t validProductAndValuesGetFirstValue(
                    const T v1, const T v2, const T min, const T max,
                    const char *msg = RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE) {

                if (areProductAndValuesWithin(v1, v2, min, max)) {
                    return v1;
                }
                throw std::invalid_argument(msg);
            }

            static constexpr size_t validSum(
                    const T v1, const T v2, const T min, const T max,
                    const char *msg = RANGE_BASE_SUM_NOT_IN_RANGE) {

                if (isSumWithin(v1, v2, min, max)) {
                    return v1 + v2;
                }
                throw std::invalid_argument(msg);
            }

            static constexpr size_t validSumGetFirstValue(
                    const T v1, const T v2, const T min, const T max,
                    const char *msg = RANGE_BASE_SUM_NOT_IN_RANGE) {

                if (isSumWithin(v1, v2, min, max)) {
                    return v1;
                }
                throw std::invalid_argument(msg);
            }

        };

        enum class LimitBaseType {
            SIZE, OFFSET
        };

        template<typename T, LimitBaseType type>
        struct LimitBase {
        };

        template<typename T>
        struct LimitBase<T, LimitBaseType::OFFSET> {

            using RangeCheck = simpledsp::algorithm::detail::RangeBase<size_t, 1>;

            static constexpr size_t maximum = std::numeric_limits<size_t>::max() / sizeof(T) - 1;

            static constexpr size_t minimum = 0;

            static constexpr size_t validOffset(
                    const size_t suggested_size, const char *msg = RANGE_BASE_VALUE_NOT_IN_RANGE) {
                return RangeCheck::validValue(
                        suggested_size, minimum, maximum, msg);
            }

            static constexpr inline size_t validOffset(
                    size_t suggested_size, size_t max, const char *msg = RANGE_BASE_VALUE_NOT_IN_RANGE) {
                return RangeCheck::validValue(
                        suggested_size, minimum, algorithm::min(maximum, max), msg);
            }

            static inline size_t validOffsetAndSize(
                    size_t offset, size_t size, const char *msg = RANGE_BASE_SUM_NOT_IN_RANGE) {
                return RangeCheck::validSum(
                        offset, size, minimum, maximum, msg);
            }

            static inline size_t validOffsetAndSize(
                    size_t offset, size_t size, size_t max, const char *msg = RANGE_BASE_SUM_NOT_IN_RANGE) {
                return RangeCheck::validProduct(
                        offset, size, minimum, algorithm::min(maximum, max), msg);
            }

            static inline size_t validOffsetAndSizeGetOffset(
                    size_t offset, size_t size, const char *msg = RANGE_BASE_SUM_NOT_IN_RANGE) {
                return RangeCheck::validProductGetFirstValue(
                        offset, size, minimum, maximum, msg);
            }

            static inline size_t validOffsetAndSizeGetOffset(
                    size_t offset, size_t size, size_t max, const char *msg = RANGE_BASE_SUM_NOT_IN_RANGE) {
                return RangeCheck::validProductGetFirstValue(
                        offset, size, minimum, algorithm::min(maximum, max), msg);
            }
        };

        template<typename T>
        struct LimitBase<T, LimitBaseType::SIZE> {

            using RangeCheck = simpledsp::algorithm::detail::RangeBase<size_t, 1>;

            static constexpr size_t maximum = std::numeric_limits<size_t>::max() / sizeof(T);

            static constexpr size_t minimum = 1;

            static constexpr size_t validSize(
                    const size_t suggested_size, const char *msg = RANGE_BASE_VALUE_NOT_IN_RANGE) {
                return RangeCheck::validValue(
                        suggested_size, minimum, maximum, msg);
            }

            static constexpr inline size_t validSize(
                    size_t suggested_size, size_t max, const char *msg = RANGE_BASE_VALUE_NOT_IN_RANGE) {
                return RangeCheck::validValue(
                        suggested_size, minimum, algorithm::min(maximum, max), msg);
            }

            static inline size_t validProduct(
                    size_t suggest1, size_t suggest2,
                    const char *msg = RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE) {
                return RangeCheck::validProductAndValues(
                        suggest1, suggest2, minimum, maximum, msg);
            }

            static inline size_t validProduct(
                    size_t suggest1, size_t suggest2, size_t max,
                    const char *msg = RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE) {
                return RangeCheck::validProductAndValues(
                        suggest1, suggest2, minimum, algorithm::min(maximum, max), msg);
            }

            static inline size_t validProductGetFirst(
                    size_t suggest1, size_t suggest2,
                    const char *msg = RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE) {
                return RangeCheck::validProductAndValuesGetFirstValue(suggest1, suggest2, minimum, maximum, msg);
            }

            static inline size_t validProductGetFirst(
                    size_t suggest1, size_t suggest2, size_t max,
                    const char *msg = RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE) {
                return RangeCheck::validProductAndValuesGetFirstValue(
                        suggest1, suggest2, minimum, algorithm::min(maximum, max), msg);
            }
        };

    }

    template<typename T>
    using RangeChecks = simpledsp::algorithm::detail::RangeBase<
            T, std::is_integral<T>::value && !std::is_signed<T>::value ? 1 : 0>;
}

#endif //SIMPLE_DSP_RANGES_H
