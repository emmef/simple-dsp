//
// Created by michel on 15-08-19.
//

#include <cstring>
#include <iostream>

#include <simple-dsp/algorithm/size.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

using namespace std;

namespace {

    struct AbstractFixedRange {
        size_t min;
        size_t max;

        AbstractFixedRange(size_t minimum, size_t maximum) : min(minimum), max(maximum) {};

        sdsp_nodiscard virtual bool isWithin(size_t value) const = 0;

        sdsp_nodiscard virtual bool isExclusiveWithin(size_t value) const = 0;

        sdsp_nodiscard virtual size_t clamp(size_t value) const = 0;

        sdsp_nodiscard virtual bool isSumWithin(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual bool areSumAndValuesWithin(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual bool isProductWithin(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual bool areProductAndValuesWithin(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validValue(size_t value) const = 0;

        sdsp_nodiscard virtual size_t validProduct(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validProductAndValues(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validProductGetFirstValue(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validProductAndValuesGetFirstValue(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validSum(size_t v1, size_t v2) const = 0;

        sdsp_nodiscard virtual size_t validSumGetFirstValue(size_t v1, size_t v2) const = 0;
    };

    struct ReferenceFixedRange : public AbstractFixedRange {
        ReferenceFixedRange(size_t min, size_t max) : AbstractFixedRange(min, max) {}

        sdsp_nodiscard bool isWithin(size_t value) const override {
            return simpledsp::algorithm::is_within<size_t>(value, min, max);
        }

        sdsp_nodiscard bool isExclusiveWithin(size_t value) const override {
            return simpledsp::algorithm::is_within_excl(value, min, max);
        }

        sdsp_nodiscard size_t clamp(size_t value) const override {
            return simpledsp::algorithm::clamp(value, min, max);
        }

        sdsp_nodiscard bool isSumWithin(size_t v1, size_t v2) const override {
            return isWithin(v1 + v2);
        }

        sdsp_nodiscard bool areSumAndValuesWithin(size_t v1, size_t v2) const override {
            return isWithin(v1) && isWithin(v2) && isWithin(v1 + v2);
        }

        sdsp_nodiscard bool isProductWithin(size_t v1, size_t v2) const override {
            return isWithin(v1 * v2);
        }

        sdsp_nodiscard bool areProductAndValuesWithin(size_t v1, size_t v2) const override {
            return isWithin(v1) && isWithin(v2) && isWithin(v1 * v2);
        }

        sdsp_nodiscard size_t validValue(size_t value) const override {

            if (isWithin(value)) {
                return value;
            }
            throw std::invalid_argument(simpledsp::algorithm::detail::RANGE_BASE_VALUE_NOT_IN_RANGE);
        }

        sdsp_nodiscard size_t validProduct(size_t v1, size_t v2) const override {

            if (isProductWithin(v1, v2)) {
                return v1 * v2;
            }

            throw std::invalid_argument(simpledsp::algorithm::detail::RANGE_BASE_PRODUCT_NOT_IN_RANGE);
        }

        sdsp_nodiscard size_t validProductAndValues(size_t v1, size_t v2) const override {

            if (areProductAndValuesWithin(v1, v2)) {
                return v1 * v2;
            }
            throw std::invalid_argument(simpledsp::algorithm::detail::RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE);
        }

        sdsp_nodiscard size_t validProductGetFirstValue(size_t v1, size_t v2) const override {

            if (isProductWithin(v1, v2)) {
                return v1;
            }
            throw std::invalid_argument(simpledsp::algorithm::detail::RANGE_BASE_PRODUCT_NOT_IN_RANGE);
        }

        sdsp_nodiscard size_t validProductAndValuesGetFirstValue(size_t v1, size_t v2) const override {

            if (areProductAndValuesWithin(v1, v2)) {
                return v1;
            }
            throw std::invalid_argument(simpledsp::algorithm::detail::RANGE_BASE_PRODUCT_AND_VALUES_NOT_IN_RANGE);
        }

        sdsp_nodiscard size_t validSum(size_t v1, size_t v2) const override {

            if (isSumWithin(v1, v2)) {
                return v1 + v2;
            }
            throw std::invalid_argument(simpledsp::algorithm::detail::RANGE_BASE_SUM_NOT_IN_RANGE);
        }

        sdsp_nodiscard size_t validSumGetFirstValue(size_t v1, size_t v2) const override {

            if (isSumWithin(v1, v2)) {
                return v1;
            }
            throw std::invalid_argument(simpledsp::algorithm::detail::RANGE_BASE_SUM_NOT_IN_RANGE);
        }

    };

    struct TestFixedRange : public AbstractFixedRange {
        TestFixedRange(size_t min, size_t max) : AbstractFixedRange(min, max) {}

        using Range = simpledsp::algorithm::RangeChecks<size_t>;

        sdsp_nodiscard bool isWithin(size_t value) const override {
            return Range::isWithin(value, min, max);
        }

        sdsp_nodiscard bool isExclusiveWithin(size_t value) const override {
            return Range::isExclusiveWithin(value, min, max);
        }

        sdsp_nodiscard size_t clamp(size_t value) const override {
            return Range::clamp(value, min, max);
        }

        sdsp_nodiscard bool isSumWithin(size_t v1, size_t v2) const override {
            return Range::isSumWithin(v1, v2, min, max);
        }

        sdsp_nodiscard bool areSumAndValuesWithin(size_t v1, size_t v2) const override {
            return Range::areSumAndValuesWithin(v1, v2, min, max);
        }

        sdsp_nodiscard bool isProductWithin(size_t v1, size_t v2) const override {
            return Range::isProductWithin(v1, v2, min, max);
        }

        sdsp_nodiscard bool areProductAndValuesWithin(size_t v1, size_t v2) const override {
            return Range::areProductAndValuesWithin(v1, v2, min, max);
        }

        sdsp_nodiscard size_t validValue(size_t value) const override {
            return Range::validValue(value, min, max);
        }

        sdsp_nodiscard size_t validProduct(size_t v1, size_t v2) const override {
            return Range::validProduct(v1, v2, min, max);
        }

        sdsp_nodiscard size_t validProductAndValues(size_t v1, size_t v2) const override {
            return Range::validProductAndValues(v1, v2, min, max);
        }

        sdsp_nodiscard size_t validProductGetFirstValue(size_t v1, size_t v2) const override {
            return Range::validProductGetFirstValue(v1, v2, min, max);
        }

        sdsp_nodiscard size_t validProductAndValuesGetFirstValue(size_t v1, size_t v2) const override {
            return Range::validProductAndValuesGetFirstValue(v1, v2, min, max);
        }

        sdsp_nodiscard size_t validSum(size_t v1, size_t v2) const override {
            return Range::validSum(v1, v2, min, max);
        }

        virtual size_t validSumGetFirstValue(
                size_t v1, size_t v2) const override {
            return Range::validSumGetFirstValue(v1, v2, min, max);
        }
    };

    struct FixedRangesPair {
        const ReferenceFixedRange reference;
        const TestFixedRange subject;

        FixedRangesPair(size_t min, size_t max) : reference(min, max), subject(min, max) {}

    };

    struct FixedRangeTest {
        const ReferenceFixedRange &reference;
        const TestFixedRange &subject;

        explicit FixedRangeTest(const FixedRangesPair &ranges) : reference(ranges.reference), subject(ranges.subject) {}

        virtual ~FixedRangeTest() = default;

        sdsp_nodiscard virtual size_t expectedValue() const = 0;

        sdsp_nodiscard virtual size_t actualValue() const = 0;

        sdsp_nodiscard virtual const char *methodName() const = 0;

        virtual void add_detail(std::ostream &output) const = 0;

        sdsp_nodiscard ssize_t effectiveExpectation() const {
            try {
                return expectedValue();
            }
            catch (const std::invalid_argument &e) {
                return -1;
            }
        }

        sdsp_nodiscard ssize_t effectiveValue() const {
            try {
                return actualValue();
            }
            catch (const std::invalid_argument &e) {
                return -1;
            }
        }

        void test() const {
            ssize_t expected = effectiveExpectation();
            ssize_t actual = effectiveValue();

            BOOST_CHECK_MESSAGE(expected == actual, *this);
        }

        void printValue(std::ostream &output, ssize_t value) const {
            if (value < 0) {
                output << "std::invalid_argument";
            } else {
                output << value;
            }
        }

        std::ostream &print(std::ostream &output) const {
            output << "RangeCheck<size_t>::" << methodName() << "(";
            add_detail(output);
            output << ", min=" << subject.min << ", max=" << subject.max << ")";

            ssize_t expected = effectiveExpectation();
            ssize_t actual = effectiveValue();

            if (expected == actual) {
                output << ": correct result(";
                printValue(output, expected);
                return output << ")";
            }
            output << ": expected(";
            printValue(output, expected);
            output << ") got (";
            printValue(output, actual);
            return output << ")";
        };
    };

    struct SingleValueFixedRangeTest : public FixedRangeTest {
        size_t v;

        SingleValueFixedRangeTest(const FixedRangesPair &ranges, size_t value) : FixedRangeTest(ranges), v(value) {}

         void add_detail(std::ostream &output) const override {
            output << "value=" << v;
        }
    };

    struct IsWithinTest : public SingleValueFixedRangeTest {
        IsWithinTest(const FixedRangesPair &ranges, size_t value) : SingleValueFixedRangeTest(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "isWithin"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.isWithin(v); }

        sdsp_nodiscard size_t actualValue() const override { return subject.isWithin(v); }
    };

    struct ValidValueTest : public SingleValueFixedRangeTest {
        ValidValueTest(const FixedRangesPair &ranges, size_t value) : SingleValueFixedRangeTest(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "validValue"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.validValue(v); }

        sdsp_nodiscard size_t actualValue() const override { return subject.validValue(v); }
    };

    struct IsExclusiveWithinTest : public SingleValueFixedRangeTest {
        IsExclusiveWithinTest(const FixedRangesPair &ranges, size_t value) : SingleValueFixedRangeTest(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "isExclusiveWithin"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.isExclusiveWithin(v); }

        sdsp_nodiscard size_t actualValue() const override { return subject.isExclusiveWithin(v); }
    };

    struct ClampTest : public SingleValueFixedRangeTest {
        ClampTest(const FixedRangesPair &ranges, size_t value) : SingleValueFixedRangeTest(ranges, value) {}

        sdsp_nodiscard const char *methodName() const override { return "clamp"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.clamp(v); }

        sdsp_nodiscard size_t actualValue() const override { return subject.clamp(v); }
    };

    struct TwoValueFixedRangeTest : public FixedRangeTest {
        size_t v1;
        size_t v2;

        TwoValueFixedRangeTest(const FixedRangesPair &ranges, size_t value1, size_t value2) : FixedRangeTest(ranges),
                                                                                              v1(value1), v2(value2) {}

         void add_detail(std::ostream &output) const override {
            output << "v1=" << v1 << ", v2=" << v2;
        }
    };

    struct ValidProductTest : public TwoValueFixedRangeTest {
        ValidProductTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : TwoValueFixedRangeTest(ranges, v1,
                                                                                                       v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validProduct"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.validProduct(v1, v2); }

        sdsp_nodiscard size_t actualValue() const override { return subject.validProduct(v1, v2); }
    };

    struct ValidProductGetFirstValueTest : public TwoValueFixedRangeTest {
        ValidProductGetFirstValueTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : TwoValueFixedRangeTest(
                ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validProductGetFirstValue"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.validProductGetFirstValue(v1, v2); }

        sdsp_nodiscard size_t actualValue() const override { return subject.validProductGetFirstValue(v1, v2); }
    };

    struct ValidProductAndValuesTest : public TwoValueFixedRangeTest {
        ValidProductAndValuesTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : TwoValueFixedRangeTest(ranges,
                                                                                                                v1,
                                                                                                                v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validProductAndValues"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.validProductAndValues(v1, v2); }

        sdsp_nodiscard size_t actualValue() const override { return subject.validProductAndValues(v1, v2); }
    };


    struct ValidProductAndValuesGetFirstValueTest : public TwoValueFixedRangeTest {
        ValidProductAndValuesGetFirstValueTest(const FixedRangesPair &ranges, size_t v1, size_t v2)
                : TwoValueFixedRangeTest(ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validProductAndValuesGetFirstValue("; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.validProductAndValuesGetFirstValue(v1, v2); }

        sdsp_nodiscard size_t actualValue() const override { return subject.validProductAndValuesGetFirstValue(v1, v2); }
    };


    struct ValidSumTest : public TwoValueFixedRangeTest {
        ValidSumTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : TwoValueFixedRangeTest(ranges, v1, v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validSum"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.validSum(v1, v2); }

        sdsp_nodiscard size_t actualValue() const override { return subject.validSum(v1, v2); }
    };

    struct ValidSumGetFirstValueTest : public TwoValueFixedRangeTest {
        ValidSumGetFirstValueTest(const FixedRangesPair &ranges, size_t v1, size_t v2) : TwoValueFixedRangeTest(ranges,
                                                                                                                v1,
                                                                                                                v2) {}

        sdsp_nodiscard const char *methodName() const override { return "validSumGetFirstValue"; }

        sdsp_nodiscard size_t expectedValue() const override { return reference.validSumGetFirstValue(v1, v2); }

        sdsp_nodiscard size_t actualValue() const override { return subject.validSumGetFirstValue(v1, v2); }
    };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
    ostream &operator<<(ostream &stream, const FixedRangeTest &s) {
        return s.print(stream);
    }
#pragma clang diagnostic pop

    std::vector<FixedRangeTest *> *generateTestCases() {
        static constexpr size_t MAX_LIMIT = 24;
        FixedRangesPair ZERO_BASED_PAIR(0, MAX_LIMIT);
        FixedRangesPair ONE_BASED_PAIR(1, MAX_LIMIT);
        FixedRangesPair GENERIC_PAIR(6, MAX_LIMIT);

        std::vector<FixedRangesPair*> ranges;
        ranges.push_back(&ZERO_BASED_PAIR);
        ranges.push_back(&ONE_BASED_PAIR);
        ranges.push_back(&GENERIC_PAIR);

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

        auto testCases = new std::vector<FixedRangeTest *>();

        for (auto range : ranges) {
            FixedRangesPair &pair = *range;
            for (size_t i : singleValues) {
                testCases->push_back(new IsWithinTest(pair, i));
                testCases->push_back(new IsExclusiveWithinTest(pair, i));
                testCases->push_back(new ClampTest(pair, i));
                testCases->push_back(new ValidValueTest(pair, i));
            }

            for (Pair &i : productValues) {
                testCases->push_back(new ValidProductTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidProductGetFirstValueTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidProductAndValuesTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidProductAndValuesGetFirstValueTest(pair, i.v1, i.v2));
            }

            for (Pair &i : sumValues) {
                testCases->push_back(new ValidSumTest(pair, i.v1, i.v2));
                testCases->push_back(new ValidSumGetFirstValueTest(pair, i.v1, i.v2));
            }
        }

        return testCases;
    }


    class TestGenerator {

        std::vector<FixedRangeTest *> *testCases;

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

        sdsp_nodiscard std::vector<FixedRangeTest *>getTestCases() const {
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
