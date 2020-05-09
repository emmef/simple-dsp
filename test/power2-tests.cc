//
// Created by michel on 18-08-19.
//

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <simple-dsp/core/alignment.h>
#include <simple-dsp/core/bounds.h>
#include <simple-dsp/core/size.h>

#include "test-helper.h"

namespace {

constexpr size_t maximumSize = std::numeric_limits<size_t>::max();

/**
 * Defines a wrapper for power of two-related implementations.
 */
struct PowerOfTwoImplementation {

  sdsp_nodiscard virtual const char *name() const = 0;

  sdsp_nodiscard virtual size_t nextOrSame(size_t size) const = 0;

  sdsp_nodiscard virtual bool is(size_t size) const = 0;

  sdsp_nodiscard virtual bool minusOne(size_t size) const = 0;

  sdsp_nodiscard virtual size_t alignedWith(size_t value,
                                            size_t power) const = 0;

  virtual ~PowerOfTwoImplementation() = default;
};

template <bool USE_CONSTEXPR>
struct SubjectImpl : public PowerOfTwoImplementation {
  using Impl = simpledsp::Power2;

  sdsp_nodiscard const char *name() const override {
    return USE_CONSTEXPR ? "Power2::constant" : "Power2";
  }

  sdsp_nodiscard size_t nextOrSame(size_t size) const override {
    return Impl::same_or_bigger(size);
  }

  sdsp_nodiscard bool is(size_t size) const override { return Impl::is(size); }

  sdsp_nodiscard bool minusOne(size_t size) const override {
    return Impl::is_minus_one(size);
  }

  sdsp_nodiscard size_t alignedWith(size_t value, size_t power) const override {
    return simpledsp::get_aligned_with(value, power);
  }
};

struct ReferenceImpl : public PowerOfTwoImplementation {

  sdsp_nodiscard const char *name() const override { return "Reference"; }

  sdsp_nodiscard size_t nextOrSame(size_t size) const override {
    if (size <= 2) {
      return 2;
    }

    for (size_t test = 2; test > 0; test *= 2) {
      if (test >= size) {
        return test;
      }
    }

    return 0;
  }

  sdsp_nodiscard bool is(size_t size) const override {
    if (size < 2) {
      return false;
    }
    for (size_t test = 2; test > 0; test *= 2) {
      if (test == size) {
        return true;
      };
    }
    return size == 1 + (maximumSize / 2);
  }

  sdsp_nodiscard bool minusOne(size_t size) const override {
    if (size == maximumSize) {
      return true;
    }
    if (size == 0) {
      return false;
    }
    for (size_t test = 2; test > 0; test *= 2) {
      if (test > size) {
        return size == test - 1;
      }
    }
    return false;
  }

  sdsp_nodiscard size_t alignedWith(size_t value, size_t power) const override {
    if (!is(power)) {
      return 0;
    }
    if (value == 0) {
      return 0;
    }
    return power * ((value + power - 1) / power);
  }

} referenceImplementation;

template <typename T, typename A, class P>
using AbstractPower2TestCase =
    simpledsp::testhelper::CompareWithReferenceTestCase<T, A, P>;

template <typename T, typename A>
class Power2TestCase
    : public AbstractPower2TestCase<T, A, PowerOfTwoImplementation> {
  const std::string name;

public:
  Power2TestCase(const PowerOfTwoImplementation &subject, A arg)
      : AbstractPower2TestCase<T, A, PowerOfTwoImplementation>(
            referenceImplementation, subject, arg),
        name(subject.name()) {}

  Power2TestCase(const PowerOfTwoImplementation &subject, A arg1, A arg2)
      : AbstractPower2TestCase<T, A, PowerOfTwoImplementation>(
            referenceImplementation, subject, arg1, arg2),
        name(subject.name()) {}

  sdsp_nodiscard const char *typeOfTestName() const override {
    return name.c_str();
  }
};

struct IsPowerTestCase : public Power2TestCase<bool, size_t> {

  IsPowerTestCase(const size_t value, const PowerOfTwoImplementation &subject)
      : Power2TestCase<bool, size_t>(subject, value) {}

  sdsp_nodiscard const char *methodName() const override { return "is"; }

  sdsp_nodiscard bool
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.is(getArgument(0));
  }
};

struct NextPowerTestCase : public Power2TestCase<size_t, size_t> {

  NextPowerTestCase(const size_t value, const PowerOfTwoImplementation &subject)
      : Power2TestCase<size_t, size_t>(subject, value) {}

  sdsp_nodiscard const char *methodName() const override {
    return "same_or_bigger";
  }

  sdsp_nodiscard size_t
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.nextOrSame(getArgument(0));
  }
};

struct IsMinusOneTestCase : public Power2TestCase<bool, size_t> {

  IsMinusOneTestCase(const size_t value,
                     const PowerOfTwoImplementation &subject)
      : Power2TestCase<bool, size_t>(subject, value) {}

  sdsp_nodiscard const char *methodName() const override {
    return "is_minus_one";
  }

  sdsp_nodiscard bool
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.minusOne(getArgument(0));
  }
};

struct AlignedWithTestCase : public Power2TestCase<size_t, size_t> {

  AlignedWithTestCase(const size_t offset, size_t power,
                      const PowerOfTwoImplementation &subject)
      : Power2TestCase<size_t, size_t>(subject, offset, power) {}

  sdsp_nodiscard const char *methodName() const override {
    return "is_aligned_with";
  }

  sdsp_nodiscard size_t
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.alignedWith(getArgument(0), getArgument(1));
  }

  sdsp_nodiscard const char *getArgumentName(size_t i) const override {
    return i == 0 ? "offset" : i == 1 ? "powerOfTwoValue" : nullptr;
  }
};

SubjectImpl<true> constant;
SubjectImpl<false> runtime;

struct TestSet {
  using TestCase = simpledsp::testhelper::AbstractValueTestCase;

  std::vector<const TestCase *> getTestCases() { return testCases; }

  TestSet() {
    std::vector<size_t> powerTestValues;
    for (size_t i = 2, j = 1; i > j; j = i, i *= 2) {
      addIfAbsent(powerTestValues, j - 1);
      addIfAbsent(powerTestValues, j);
      addIfAbsent(powerTestValues, j + 1);
    }
    addIfAbsent(powerTestValues, std::numeric_limits<size_t>::max() - 1);
    addIfAbsent(powerTestValues, std::numeric_limits<size_t>::max());

    for (size_t value : powerTestValues) {
      testCases.emplace_back(new IsPowerTestCase(value, runtime));
      testCases.emplace_back(new IsPowerTestCase(value, constant));
      testCases.emplace_back(new IsMinusOneTestCase(value, runtime));
      testCases.emplace_back(new IsMinusOneTestCase(value, constant));
      testCases.emplace_back(new NextPowerTestCase(value, runtime));
      testCases.emplace_back(new NextPowerTestCase(value, constant));
    }
    for (size_t offset : powerTestValues) {
      for (size_t powerOfTwo : powerTestValues) {
        if (offset < 10000000 && powerOfTwo < 128 &&
            referenceImplementation.is(powerOfTwo)) {
          testCases.emplace_back(
              new AlignedWithTestCase(offset, powerOfTwo, runtime));
          testCases.emplace_back(
              new AlignedWithTestCase(offset, powerOfTwo, constant));
        }
      }
    }
  }

  ~TestSet() {
    for (const TestCase *testCase : testCases) {
      delete testCase;
    }
  }

private:
  std::vector<const TestCase *> testCases;

  static void addIfAbsent(std::vector<size_t> &values, size_t value) {
    if (!contains(values, value)) {
      values.push_back(value);
    }
  }

  static bool contains(const std::vector<size_t> &haystack, size_t needle) {
    for (size_t value : haystack) {
      if (value == needle) {
        return true;
      }
    }
    return false;
  }
} TEST_SET;

struct MsbTestScenario {
  size_t input;
  int expected;
  int (*function)(size_t);
  const char *name;

  static MsbTestScenario most_significant_bit(size_t value, int expected) {
    return {value, expected, simpledsp::Bits<size_t>::most_significant,
            "PowerTwo::most_significant_bit"};
  }

  static MsbTestScenario most_significant_single_bit(size_t value,
                                                     int expected) {
    return {value, expected,
            simpledsp::Bits<size_t>::most_significant_single,
            "PowerTwo::most_significant_single_bit"};
  }

  bool success() const { return expected == function(input); }

  void print(std::ostream &out) const {
    out << name << "(" << input << ")";
    if (success()) {
      out << " = " << expected;
    } else {
      out << " = " << function(input) << ", but expected " << expected;
    }
  }
};

std::ostream &operator<<(std::ostream &stream, const MsbTestScenario &s) {
  s.print(stream);
  return stream;
}

class MsbTestCases {
  std::vector<MsbTestScenario> testCases;

public:
  MsbTestCases() {

    testCases.push_back(MsbTestScenario::most_significant_bit(0, -1));
    testCases.push_back(MsbTestScenario::most_significant_single_bit(0, -1));

    testCases.push_back(MsbTestScenario::most_significant_bit(1, 0));
    testCases.push_back(MsbTestScenario::most_significant_single_bit(1, 0));

    testCases.push_back(MsbTestScenario::most_significant_bit(2, 1));
    testCases.push_back(MsbTestScenario::most_significant_single_bit(2, 1));

    testCases.push_back(MsbTestScenario::most_significant_bit(4, 2));
    testCases.push_back(MsbTestScenario::most_significant_single_bit(4, 2));

    int maxbit = sizeof(size_t) * 8 - 1;
    size_t max = size_t(1) << maxbit;

    testCases.push_back(MsbTestScenario::most_significant_bit(max, maxbit));
    testCases.push_back(
        MsbTestScenario::most_significant_single_bit(max, maxbit));

    testCases.push_back(MsbTestScenario::most_significant_bit(0x10, 4));
    testCases.push_back(MsbTestScenario::most_significant_single_bit(0x10, 4));

    testCases.push_back(MsbTestScenario::most_significant_bit(0x11, 4));
    testCases.push_back(MsbTestScenario::most_significant_single_bit(0x11, -1));

    testCases.push_back(MsbTestScenario::most_significant_bit(0x12, 4));
    testCases.push_back(MsbTestScenario::most_significant_single_bit(0x12, -2));
  }

  const std::vector<MsbTestScenario> getTestCases() { return testCases; }

} MSB_TESTCASES;

} // namespace

BOOST_AUTO_TEST_SUITE(PowerOfTwo)

BOOST_DATA_TEST_CASE(powerTwoScenarios, TEST_SET.getTestCases()) {
  sample->test();
}

BOOST_DATA_TEST_CASE(msbScenarios, MSB_TESTCASES.getTestCases()) {
  BOOST_CHECK(sample.success());
}

BOOST_AUTO_TEST_SUITE_END()
