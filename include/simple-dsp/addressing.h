#ifndef SIMPLE_DSP_ADDRESSING_H
#define SIMPLE_DSP_ADDRESSING_H
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

#include <stdexcept>
#include <simple-dsp/power2.h>
#include <simple-dsp/algorithm.h>

namespace simpledsp {

#if SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS > 0
    static constexpr size_t SDSP_MEMORY_MODEL_STOLEN_ADDRESS_BITS =
            ::std::clamp(size_t(SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS), size_t(1), sizeof(size_t) * 8);
#else
    /*
     * This is really guesswork. On 32-bit and smaller systems, most memory tends
     * to be available for each process. On larger systems, some bits are often
     * reserved to distinguish privilege levels or additional paging. In the case
     * of Intel, this contributes to 17 bits on 64-bit architectures. This idea of
     * one 4th plus one of the bits being reserved for OS/hardware stuff is
     * extrapolated.
     * Suggestions to make these guesses more accurate in a platform independent
     * way are welcome, or feel free to override the maximum by defining
     * SDSP_OVERRIDE_MEMORY_MODEL_STOLEN_ADDRESS_BITS.
     */
    static constexpr size_t SDSP_MEMORY_MODEL_STOLEN_ADDRESS_BITS =
            sizeof(size_t) > 4 ? 1 + 8*(sizeof(size_t)/4) : 1;
#endif
    static constexpr size_t SDSP_MAX_ALLOC_SIZE = (~size_t(0)) >> SDSP_MEMORY_MODEL_STOLEN_ADDRESS_BITS;

    enum class ValidGet { FIRST, RESULT };

    template<typename T, size_t customMaximum = 0>
    struct AddressRange {
        static constexpr size_t absoluteMaximum =
                (SDSP_MAX_ALLOC_SIZE - alignof(T)) / sizeof(T);

        static constexpr size_t maximum = is_within_excl<size_t>(
                customMaximum, 0, absoluteMaximum) ? customMaximum : absoluteMaximum;

        static constexpr bool isValidRelaxed(size_t value) {
            return value <= maximum;
        }

        static constexpr bool isValidStrict(size_t value) {
            return value && isValidRelaxed(value);
        }

        /**
         * Returns whether the sum of two values does not exceed the maximum that
         * is allowed for the type. This can be used to verify a valid combination
         * of sizes and offsets.
         *
         * @param value1 The first value
         * @param value2 The second value
         * @return {@code true} if the sum does not exceed the maximum.
         */
        static constexpr bool isValidSumRelaxed(size_t value1, size_t value2) {
            return value1 <= maximum && maximum - value1 >= value2;
        }

        /**
         * Returns whether the product of two values does not exceed the maximum that
         * is allowed for the type. Both arguments are allowed to be zero.
         *
         * @param value1 The first value
         * @param value2 The second value
         * @return {@code true} if the product does not exceed the maximum.
         */
        static constexpr bool isValidProductRelaxed(size_t value1, size_t value2) {
            return (value1 > 0 && maximum / value1 >= value2) || value2 < maximum;
        }

        /**
         * Returns whether the product of two values does not exceed the maximum that
         * is allowed for the type. Both arguments are not allowed to be zero.
         *
         * @param value1 The first value that cannot be zero
         * @param value2 The second value that cannot be zero
         * @return {@code true} if the product does not exceed the maximum.
         */
        static constexpr bool isValidProductStrict(size_t value1, size_t value2) {
            return value1 && value2 && maximum / value1 >= value2;
        }

        /**
         * Returns whether the sum of two values does not exceed the maximum that
         * is allowed for the type. Both values cannot be zero. This can be used
         * to verify a combination of sizes.
         *
         * @param value1 The first value, that cannot be zero.
         * @param value2 The second value, that cannot be zero.
         * @return {@code true} if the sum does not exceed the maximum.
         */
        static constexpr bool isValidSumStrict(size_t value1, size_t value2) {
            return value1 && value2 && isValidSumRelaxed(value1, value2);
        }

        static size_t validStrictGet(size_t value) {
            if (isValidStrict(value)) {
                return value;
            }
            throw std::invalid_argument(
                    "Size::get_valid_product: value exceeds exceeds maximum addressing range for type or is zero.");
        }

        static size_t validRelaxedGet(size_t value) {
            if (isValidRelaxed(value)) {
                return value;
            }
            throw std::invalid_argument(
                    "Size::get_valid_product: value exceeds exceeds maximum addressing range.");
        }

        /**
         * If the result is valid according to isValidProductStrict, return the first
         * value or the result according to validGet; otherwise, throw an invalid
         * argument exception.
         *
         * @see isValidProductStrict
         * @param value1 The first value
         * @param value2 The second value
         * @param validGet Indicates whether the first value or result is returned if result is valid.
         * @return the first value or result, based on the value of validGet.
         */
        static size_t validProductStrictGet(size_t value1, size_t value2, ValidGet validGet) {
            if (isValidProductStrict(value1, value2)) {
                return validGet == ValidGet::FIRST ? value1 : value1 * value2;
            }
            throw std::invalid_argument(
                    "Size::get_valid_product: product of values exceeds maximum addressing range for type or one of the sizes is zero.");
        }

        /**
         * If the result is valid according to isValidProductRelaxed, return the first
         * value or the result according to validGet; otherwise, throw an invalid
         * argument exception.
         *
         * @see isValidProductRelaxed
         * @param value1 The first value
         * @param value2 The second value
         * @param validGet Indicates whether the first value or result is returned if result is valid.
         * @return the first value or result, based on the value of validGet.
         */
        static size_t validProductRelaxedGet(size_t value1, size_t value2, ValidGet validGet) {
            if (isValidProductRelaxed(value1, value2)) {
                return validGet == ValidGet::FIRST ? value1 : value1 * value2;
            }
            throw std::invalid_argument(
                    "Size::get_valid_product: product of values exceeds exceeds maximum addressing range for type.");
        }

        /**
         * If the result is valid according to isValidSumStrict, return the first
         * value or the result according to validGet; otherwise, throw an invalid
         * argument exception.
         *
         * @see isValidSumStrict
         * @param value1 The first value
         * @param value2 The second value
         * @param validGet Indicates whether the first value or result is returned if result is valid.
         * @return the first value or result, based on the value of validGet.
         */
        static size_t validSumStrictGet(size_t value1, size_t value2, ValidGet validGet) {
            if (isValidSumStrict(value1, value2)) {
                return validGet == ValidGet::FIRST ? value1 : value1 * value2;
            }
            throw std::invalid_argument(
                    "Size::get_valid_product: sum of values exceeds exceeds maximum addressing range for type  or one of the values is zero.");
        }

        /**
         * If the result is valid according to isValidSumRelaxed, return the first
         * value or the result according to validGet; otherwise, throw an invalid
         * argument exception.
         *
         * @see isValidSumRelaxed
         * @param value1 The first value
         * @param value2 The second value
         * @param validGet Indicates whether the first value or result is returned if result is valid.
         * @return the first value or result, based on the value of validGet.
         */
        static size_t validSumRelaxedGet(size_t value1, size_t value2, ValidGet validGet) {
            if (isValidSumRelaxed(value1, value2)) {
                return validGet == ValidGet::FIRST ? value1 : value1 * value2;
            }
            throw std::invalid_argument(
                    "Size::get_valid_product: product of values exceeds exceeds maximum addressing range for type.");
        }

    };

    template<typename T, size_t customMaximum = 0>
    struct Size {
        using Range = AddressRange<T, customMaximum>;
        static constexpr size_t absoluteMaximum = Range::absoluteMaximum;
        static constexpr size_t maximum = Range::maximum;

        static constexpr size_t isValid(size_t value) {
            return Range::isValidStrict(value);
        }

        static constexpr size_t isValidProduct(size_t value1, size_t value2) {
            return Range::isValidProductStrict(value1, value2);
        }

        static constexpr size_t isValidSum(size_t value1, size_t value2) {
            return Range::isValidSumStrict(value1, value2);
        }

        static size_t validGet(size_t value) {
            return Range::validStrictGet(value);
        }

        static size_t validProductGet(size_t value1, size_t value2, ValidGet validGet) {
            return Range::validProductStrictGet(value1, value2, validGet);
        }

        static size_t validSumGet(size_t value1, size_t value2, ValidGet validGet) {
            return Range::validSumStrictGet(value1, value2, validGet);
        }
    };

    template <typename T, size_t customMaximum = 0>
    struct Offset {
        using Range = AddressRange<T, customMaximum>;
        static constexpr size_t absoluteMaximum = Range::absoluteMaximum;
        static constexpr size_t maximum = Range::maximum;

        static constexpr size_t isValid(size_t value) {
            return Range::isValidRelaxed(value);
        }

        static constexpr size_t isValidProduct(size_t value1, size_t value2) {
            return Range::isValidProductRelaxed(value1, value2);
        }

        static constexpr size_t isValidSum(size_t value1, size_t value2) {
            return Range::isValidSumRelaxed(value1, value2);
        }

        static size_t validGet(size_t value) {
            return Range::validRelaxedGet(value);
        }

        static constexpr size_t validProductGet(size_t value1, size_t value2, ValidGet validGet) {
            return Range::validProductRelaxedGet(value1, value2, validGet);
        }

        static constexpr size_t validSumGet(size_t value1, size_t value2, ValidGet validGet) {
            return Range::validSumRelaxedGet(value1, value2, validGet);
        }
    };

    namespace base {
        namespace {

            enum class IndexPolicyType {
                THROW,
                WRAP,
                UNCHECKED
            };

            template <typename SizeType, IndexPolicyType type>
            struct IndexPolicyBase;

            template<typename SizeType>
            struct IndexPolicyBase<SizeType, IndexPolicyType::THROW> {
                sdsp_nodiscard static SizeType index(SizeType i, SizeType size) {
                    if (i < size) {
                        return i;
                    }
                    throw std::invalid_argument("IndexPolicy::index out of range");
                }
                sdsp_nodiscard static SizeType offset(SizeType o, SizeType maxOffset) {
                    if (o <= maxOffset) {
                        return o;
                    }
                    throw std::invalid_argument("IndexPolicy::offset out of range");
                }
            };

            template<typename SizeType>
            struct IndexPolicyBase<SizeType, IndexPolicyType::WRAP> {
                sdsp_nodiscard sdsp_force_inline static SizeType index(SizeType i, SizeType size) {
                    return i % size;
                }
                sdsp_nodiscard sdsp_force_inline static SizeType offset(SizeType o, SizeType maxOffset) {
                    return o % (maxOffset + 1);
                }
            };

            template<typename SizeType>
            struct IndexPolicyBase<SizeType, IndexPolicyType::UNCHECKED> {
                sdsp_nodiscard sdsp_force_inline static SizeType index(SizeType i, SizeType) {
                    return i;
                }
                sdsp_nodiscard sdsp_force_inline static SizeType offset(SizeType o, SizeType) {
                    return o;
                }
            };

        }
    }

    template<typename S>
    struct IndexBase {
#ifndef SDSP_INDEX_POLICY_METHODS_UNCHECKED
        using Method = base::IndexPolicyBase<S, base::IndexPolicyType::THROW>;
#else
        using Method = IndexPolicyBase<S, IndexPolicyType::UNCHECKED>;
#endif
#ifndef SDSP_INDEX_POLICY_ARRAYS_CHECKED
        using Array = base::IndexPolicyBase<S, base::IndexPolicyType::UNCHECKED>;
#else
        using Array = IndexPolicyBase<S, IndexPolicyType::THROW>;
#endif

        using Throw = base::IndexPolicyBase<S, base::IndexPolicyType::THROW>;

        using Wrap = base::IndexPolicyBase<S, base::IndexPolicyType::WRAP>;

        using Unchecked = base::IndexPolicyBase<S, base::IndexPolicyType::UNCHECKED>;

        sdsp_nodiscard static S arrayIndex(S i, S size) { return Array::index(i, size); }

        sdsp_nodiscard static S arrayOffset(S i, S maxOffset) { return Array::offset(i, maxOffset); }

        sdsp_nodiscard static S index(S i, S size) { return Method::index(i, size); }

        sdsp_nodiscard static S offset(S i, S maxOffset) { return Method::offset(i, maxOffset); }
    };

    using Index = IndexBase<size_t>;


} // namespace simpledsp::algorithm



#endif //SIMPLE_DSP_ADDRESSING_H
