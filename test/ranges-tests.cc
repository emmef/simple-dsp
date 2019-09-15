//
// Created by michel on 15-08-19.
//

#include <cstring>
#include <iostream>

#include <simple-dsp/addressing.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "test-helper.h"

using namespace std;

namespace {

    struct AbstractFixedRange {
        size_t max;

        AbstractFixedRange(size_t maximum) : max(maximum) {};

        sdsp_nodiscard virtual bool isWithin(size_t value) const = 0;

        sdsp_nodiscard virtual bool isWithinExclusive(size_t value) const = 0;

        sdsp_nodiscard virtual size_t clamp(size_t value) const = 0;

        sdsp_nodiscard virtual bool isSumWithin(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual bool isSizeSumWithin(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual bool isProductWithin(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual bool isSizeProductWithin(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validValue(size_t value) const = 0;

        sdsp_nodiscard virtual size_t validSizeValue(size_t value) const = 0;

        sdsp_nodiscard virtual size_t validProduct(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validSizeProduct(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validProductGetFirst(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validSizeProductGetFirst(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validSum(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validSumGetFirst(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validSizeSum(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validSizeSumGetFirst(size_t v1, size_t v2) const = 0;
    };

    struct ReferenceFixedRange : public AbstractFixedRange {
        ReferenceFixedRange(size_t max) : AbstractFixedRange(max) {}

        sdsp_nodiscard bool isWithin(size_t value) const override {
            return value <= max;
        }

        sdsp_nodiscard bool isWithinExclusive(size_t value) const override {
            return value > 0 && value < max;
        }

        sdsp_nodiscard size_t clamp(size_t value) const override {
            return value >= max ? max : value;
        }

        sdsp_nodiscard bool isSumWithin(size_t v1, size_t v2) const override {
            return isWithin(v1 + v2);
        }

        sdsp_nodiscard bool isSizeSumWithin(size_t v1, size_t v2) const override {
            return v1 > 0 && v2 > 0 && isWithin(v1 + v2);
        }

        sdsp_nodiscard bool isProductWithin(size_t v1, size_t v2) const override {
            return isWithin(v1 * v2);
        }

        sdsp_nodiscard bool isSizeProductWithin(size_t v1, size_t v2) const override {
            return v1 > 0 && v2 > 0 && isWithin(v1 * v2);
        }

        sdsp_nodiscard size_t validValue(size_t value) const override {

            if (isWithin(value)) {
                return value;
            }
            throw std::invalid_argument("Value not in range");
        }
        sdsp_nodiscard size_t validSizeValue(size_t value) const override  {
            if (value > 0 && isWithin(value)) {
                return value;
            }
            throw std::invalid_argument("Value not in range");
        };

        sdsp_nodiscard size_t validProduct(size_t v1, size_t v2) const override {

            if (isProductWithin(v1, v2)) {
                return v1 * v2;
            }

            throw std::invalid_argument("product not in range");
        }

        sdsp_nodiscard size_t validSizeProduct(size_t v1, size_t v2) const override {

            if (isSizeProductWithin(v1, v2)) {
                return v1 * v2;
            }
            throw std::invalid_argument("product and values not in range");
        }

        sdsp_nodiscard size_t validProductGetFirst(size_t v1, size_t v2) const override {

            if (isProductWithin(v1, v2)) {
                return v1;
            }
            throw std::invalid_argument("product not in range");
        }

        sdsp_nodiscard size_t validSizeProductGetFirst(size_t v1, size_t v2) const override {

            if (isSizeProductWithin(v1, v2)) {
                return v1;
            }
            throw std::invalid_argument("product and values not in range");
        }

        sdsp_nodiscard size_t validSum(size_t v1, size_t v2) const override {

            if (isSumWithin(v1, v2)) {
                return v1 + v2;
            }
            throw std::invalid_argument("sum not in range");
        }

        sdsp_nodiscard size_t validSumGetFirst(size_t v1, size_t v2) const override {

            if (isSumWithin(v1, v2)) {
                return v1;
            }
            throw std::invalid_argument("sum not in range");
        }

        sdsp_nodiscard size_t validSizeSum(size_t v1, size_t v2) const override {
            if (isSizeSumWithin(v1, v2)) {
                return v1 + v2;
            }
            throw std::invalid_argument("sum not in range");
        }

        sdsp_nodiscard size_t validSizeSumGetFirst(size_t v1, size_t v2) const override {
            if (isSizeSumWithin(v1, v2)) {
                return v1;
            }
            throw std::invalid_argument("sum not in range");
        }

    };

    template<size_t MAX>
    struct TestFixedRange : public AbstractFixedRange {
        TestFixedRange() : AbstractFixedRange(MAX) {}

        sdsp_nodiscard bool isWithin(size_t value) const override {
            return simpledsp::is_within(value, size_t(0), max);
        }

        sdsp_nodiscard bool isWithinExclusive(size_t value) const override {
            return simpledsp::is_within_excl(value, size_t(0), max);
        }

        sdsp_nodiscard size_t clamp(size_t value) const override {
            return ::std::clamp(value, size_t(0), max);
        }

        sdsp_nodiscard bool isSumWithin(size_t v1, size_t v2) const override {
            return simpledsp::Offset<size_t, MAX>::isValidSum(v1, v2);
        }

        sdsp_nodiscard bool isSizeSumWithin(size_t v1, size_t v2) const override {
            return simpledsp::Size<size_t, MAX>::isValidSum(v1, v2);
        }

        sdsp_nodiscard bool isProductWithin(size_t v1, size_t v2) const override {
            return simpledsp::Offset<size_t, MAX>::isValidProduct(v1, v2);
        }

        sdsp_nodiscard bool isSizeProductWithin(size_t v1, size_t v2) const override {
            return simpledsp::Size<size_t, MAX>::isValidProduct(v1, v2);
        }

        sdsp_nodiscard size_t validValue(size_t value) const override {
            return simpledsp::Offset<size_t, MAX>::validGet(value);
        }

        sdsp_nodiscard size_t validSizeValue(size_t value) const override {
            return simpledsp::Size<size_t, MAX>::validGet(value);
        }

        sdsp_nodiscard size_t validProduct(size_t v1, size_t v2) const override {
            return simpledsp::Offset<size_t, MAX>::validProductGet(v1, v2, simpledsp::ValidGet::RESULT);
        }

        sdsp_nodiscard size_t validSizeProduct(size_t v1, size_t v2) const override {
            return simpledsp::Size<size_t, MAX>::validProductGet(v1, v2, simpledsp::ValidGet::RESULT);
        }

        sdsp_nodiscard size_t validProductGetFirst(size_t v1, size_t v2) const override {
            return simpledsp::Offset<size_t, MAX>::validProductGet(v1, v2, simpledsp::ValidGet::FIRST);
        }

        sdsp_nodiscard size_t validSizeProductGetFirst(size_t v1, size_t v2) const override {
            return simpledsp::Size<size_t, MAX>::validProductGet(v1, v2, simpledsp::ValidGet::FIRST);
        }

        sdsp_nodiscard size_t validSum(size_t v1, size_t v2) const override {
            return simpledsp::Offset<size_t, MAX>::validSumGet(v1, v2, simpledsp::ValidGet::RESULT);
        }

        sdsp_nodiscard size_t validSumGetFirst(size_t v1, size_t v2) const override {
            return simpledsp::Offset<size_t, MAX>::validSumGet(v1, v2, simpledsp::ValidGet::FIRST);
        }

        sdsp_nodiscard size_t validSizeSum(size_t v1, size_t v2) const override {
            return simpledsp::Size<size_t,MAX>::validSumGet(v1, v2, simpledsp::ValidGet::RESULT);
        }

        sdsp_nodiscard size_t validSizeSumGetFirst(size_t v1, size_t v2) const override {
            return simpledsp::Size<size_t,MAX>::validSumGet(v1, v2, simpledsp::ValidGet::FIRST);
        }

    };

    struct FixedRangesPair {
        const ReferenceFixedRange reference;
        static constexpr size_t MAX_LIMIT = 24;
        const TestFixedRange<MAX_LIMIT> subject;

        FixedRangesPair(size_t max) : reference(max), subject() {}
    }
    const predefinedRanges[1] = {
            {FixedRangesPair::MAX_LIMIT}};

    template<typename T, typename A, class P>
    using ValueTestCase = simpledsp::testhelper::ValueTestCase<T, A, P>;

    template<typename Result>
    struct FixedRangeTest : public ValueTestCase<Result, size_t, AbstractFixedRange> {
        FixedRangeTest(const FixedRangesPair &impls, size_t value) : ValueTestCase<Result, size_t, AbstractFixedRange>(
                impls.reference, impls.subject, value) {}
        FixedRangeTest(const FixedRangesPair &impls, size_t v1, size_t v2) : ValueTestCase<Result, size_t, AbstractFixedRange>(
                impls.reference, impls.subject, v1, v2) {}

        sdsp_nodiscard const char * typeOfTestName() const override {
            return "FixedRangeTest";
        };
    };

    struct IsWithinTest : public FixedRangeTest<bool> {
        IsWithinTest(const FixedRangesPair &ranges, size_t value) : FixedRangeTest<bool>(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "isWithin"; }

        sdsp_nodiscard bool generateValue(
                const AbstractFixedRange &impl) const override {
            return impl.isWithin(getArgument(0));
        }
    };

    struct ValidValueTest : public FixedRangeTest<size_t> {
        ValidValueTest(const FixedRangesPair &ranges, size_t value) :FixedRangeTest<size_t>(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "validValue"; }

        sdsp_nodiscard size_t generateValue(
                const AbstractFixedRange &impl) const override {
            return impl.validValue(getArgument(0));
        }
    };

    struct ValidSizeValueTest : public FixedRangeTest<size_t> {
        ValidSizeValueTest(const FixedRangesPair &ranges, size_t value) :FixedRangeTest<size_t>(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "validValue"; }

        sdsp_nodiscard size_t generateValue(
                const AbstractFixedRange &impl) const override {
            return impl.validSizeValue(getArgument(0));
        }
    };

    struct IsExclusiveWithinTest : public FixedRangeTest<bool> {
        IsExclusiveWithinTest(const FixedRangesPair &ranges, size_t value) : FixedRangeTest<bool>(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "isWithinExclusive"; }

        sdsp_nodiscard bool generateValue(
                const AbstractFixedRange &impl) const override {
            return impl.isWithinExclusive(getArgument(0));
        }
    };

    struct ClampTest : public FixedRangeTest<size_t> {
        ClampTest(const FixedRangesPair &ranges, size_t value) : FixedRangeTest<size_t>(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "clamp"; }

        sdsp_nodiscard size_t generateValue(const AbstractFixedRange &impl) const override {
            return impl.clamp(getArgument(0));
        }
    };

    struct IsValidProductTest : public FixedRangeTest<bool> {
        IsValidProductTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : FixedRangeTest<bool>(ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "isProductWithin"; }

        sdsp_nodiscard bool generateValue(const AbstractFixedRange &impl) const override {
            return impl.isProductWithin(getArgument(0), getArgument(1));
        }
    };

    struct ValidProductTest : public FixedRangeTest<size_t> {
        ValidProductTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : FixedRangeTest<size_t>(ranges, v1,
                                                                                                       v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validProduct"; }

        sdsp_nodiscard size_t generateValue(
                const AbstractFixedRange &impl) const override {
            return impl.validProduct(getArgument(0), getArgument(1));
        }
    };

    struct ValidProductGetFirstValueTest : public FixedRangeTest<size_t> {
        ValidProductGetFirstValueTest(const FixedRangesPair &ranges, size_t v1, size_t v2) :
        FixedRangeTest<size_t>(
                ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validProductGetFirst"; }

        sdsp_nodiscard size_t generateValue(const AbstractFixedRange &impl) const override {
            return impl.validProductGetFirst(getArgument(0), getArgument(1));
        }
    };

    struct ValidProductAndValuesTest : public FixedRangeTest<size_t> {
        ValidProductAndValuesTest(const FixedRangesPair &ranges, size_t v1, size_t v2) :
        FixedRangeTest<size_t>(ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validSizeProduct"; }

        sdsp_nodiscard size_t generateValue(const AbstractFixedRange &impl) const override {
            return impl.validSizeProduct(getArgument(0), getArgument(1));
        }
    };

    struct ValidProductAndValuesGetFirstValueTest : public FixedRangeTest<size_t> {

        ValidProductAndValuesGetFirstValueTest(const FixedRangesPair &ranges, size_t v1, size_t v2) :
                FixedRangeTest<size_t>(ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validSizeProductGetFirst"; }

        sdsp_nodiscard size_t generateValue(const AbstractFixedRange &impl) const override {
            return impl.validSizeProductGetFirst(getArgument(0), getArgument(1));
        }
    };

    struct IsValidSumTest : public FixedRangeTest<bool> {
        IsValidSumTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : FixedRangeTest<bool>(ranges, v1,
                                                                                                   v2) {}

        sdsp_nodiscard const char *methodName() const override { return "isSumWithin"; }

        sdsp_nodiscard bool generateValue(
                const AbstractFixedRange &impl) const override {
            return impl.isSumWithin(getArgument(0), getArgument(1));
        }
    };

    struct ValidSumTest : public FixedRangeTest<size_t> {
        ValidSumTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : FixedRangeTest<size_t>(ranges, v1,
                                                                                                   v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validSum"; }

        sdsp_nodiscard size_t generateValue(
                const AbstractFixedRange &impl) const override {
            return impl.validSum(getArgument(0), getArgument(1));
        }
    };

    struct ValidSumGetFirstValueTest : public FixedRangeTest<size_t> {
        ValidSumGetFirstValueTest(const FixedRangesPair &ranges, size_t v1, size_t v2) :
                FixedRangeTest<size_t>(
                        ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validSumGetFirst"; }

        sdsp_nodiscard size_t generateValue(const AbstractFixedRange &impl) const override {
            return impl.validSumGetFirst(getArgument(0), getArgument(1));
        }
    };

    struct ValidSizeSumTest : public FixedRangeTest<size_t> {
        ValidSizeSumTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : FixedRangeTest<size_t>(ranges, v1,
                                                                                                   v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validSizeSum"; }

        sdsp_nodiscard size_t generateValue(
                const AbstractFixedRange &impl) const override {
            return impl.validSizeSum(getArgument(0), getArgument(1));
        }
    };

    struct ValidSizeSumGetFirstValueTest : public FixedRangeTest<size_t> {
        ValidSizeSumGetFirstValueTest(const FixedRangesPair &ranges, size_t v1, size_t v2) :
                FixedRangeTest<size_t>(
                        ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validSizeSumGetFirst"; }

        sdsp_nodiscard size_t generateValue(const AbstractFixedRange &impl) const override {
            return impl.validSizeSumGetFirst(getArgument(0), getArgument(1));
        }
    };

    std::vector<simpledsp::testhelper::AbstractValueTestCase *> *generateTestCases() {
        constexpr size_t MAX_LIMIT = FixedRangesPair::MAX_LIMIT;

        std::vector<size_t> singleValues;
        singleValues.push_back(0);
        singleValues.push_back(1);
        singleValues.push_back(2);
        singleValues.push_back(3);
        singleValues.push_back(MAX_LIMIT - 1);
        singleValues.push_back(MAX_LIMIT);
        singleValues.push_back(MAX_LIMIT + 1);

        struct Pair { size_t v1; size_t v2; };

        std::vector<Pair> productValues;
        for (size_t i = 0; i <= MAX_LIMIT + 1; i++) {
            for (size_t j = i; j <= MAX_LIMIT + 1; j++) {
                size_t product = j * i;
                if (product < 4 || (product > MAX_LIMIT -2 && product < MAX_LIMIT + 2)) {
                    productValues.push_back({i, j});
                    productValues.push_back({j, i});
                }
            }
        }

        std::vector<Pair> sumValues;
        for (size_t i = 0; i <= MAX_LIMIT + 1; i++) {
            for (size_t j = i; j <= MAX_LIMIT + 1; j++) {
                size_t sum = j + i;
                if (sum < 4 || (sum > MAX_LIMIT - 2 && sum < MAX_LIMIT + 2)) {
                    productValues.push_back({i, j});
                    productValues.push_back({j, i});
                }
            }
        }

        auto testCases = new std::vector<simpledsp::testhelper::AbstractValueTestCase *>();

        for (const auto & pair : predefinedRanges) {
            for (size_t i : singleValues) {
                testCases->push_back(new IsWithinTest(pair, i));
                testCases->push_back(new IsExclusiveWithinTest(pair, i));
                testCases->push_back(new ClampTest(pair, i));
                testCases->push_back(new ValidValueTest(pair, i));
                testCases->push_back(new ValidSizeValueTest(pair, i));
            }

            for (Pair &i : productValues) {
                testCases->push_back(new IsValidProductTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidProductTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidProductGetFirstValueTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidProductAndValuesTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidProductAndValuesGetFirstValueTest(pair, i.v1, i.v2));
            }

            for (Pair &i : sumValues) {
                testCases->push_back(new IsValidSumTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidSumTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidSumGetFirstValueTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidSizeSumTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidSizeSumGetFirstValueTest(pair, i.v1, i.v2));
            }
        }

        return testCases;
    }


    class TestGenerator {

        std::vector<simpledsp::testhelper::AbstractValueTestCase *> *testCases;

    public:
        TestGenerator() {
            testCases = generateTestCases();
        }

        ~TestGenerator() {
            if (testCases) {
                for (auto testCase : *testCases) {
                    delete testCase;
                }
                delete testCases;
                testCases = nullptr;
            }
        }

        sdsp_nodiscard std::vector<simpledsp::testhelper::AbstractValueTestCase *>getTestCases() const {
            return *testCases;
        }

    } TEST_GENERATOR;
}


BOOST_AUTO_TEST_SUITE(testRanges)


BOOST_DATA_TEST_CASE(sample, TEST_GENERATOR.getTestCases())
{
    sample->test();
}

BOOST_AUTO_TEST_SUITE_END()
