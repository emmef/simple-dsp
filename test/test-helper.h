#ifndef SIMPLE_DSP_TEST_HELPER_H
#define SIMPLE_DSP_TEST_HELPER_H
/*
 * simple-dsp/test-helper.h
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

#include <array>
#include <iostream>
#include <simple-dsp/attributes.h>

namespace simpledsp::testhelper {

namespace {
struct AbstractValueTestCase {

  virtual void test() const = 0;

  virtual std::ostream &print(std::ostream &stream) const = 0;

  virtual ~AbstractValueTestCase() = default;
};

template <typename T, typename A, class TestInterface>
class ValueTestCase : public AbstractValueTestCase {
  const TestInterface &expectedValues;
  const TestInterface &actualValues;
  const size_t count;
  const std::array<A, 3> arguments;

  sdsp_nodiscard bool effectiveValue(T &result,
                                     const TestInterface &values) const {
    try {
      result = generateValue(values);
      return false;
    } catch (const std::exception &) {
      return true;
    }
  }

  void printValue(std::ostream &stream, const T &value, bool thrown) const {
    if (thrown) {
      stream << "std::exception";
    } else {
      stream << value;
    }
  }

protected:
  A getArgument(size_t i) const { return arguments.at(i); }

public:
  ValueTestCase(const TestInterface &expected, const TestInterface &actual)
      : expectedValues(expected), actualValues(actual), count(1) {}

  ValueTestCase(const TestInterface &expected, const TestInterface &actual,
                A value)
      : expectedValues(expected), actualValues(actual), count(1),
        arguments({value, value, value}) {}

  ValueTestCase(const TestInterface &expected, const TestInterface &actual,
                A v1, A v2)
      : expectedValues(expected), actualValues(actual), count(2),
        arguments({v1, v2, v1}) {}

  ValueTestCase(const TestInterface &expected, const TestInterface &actual,
                A v1, A v2, A v3)
      : expectedValues(expected), actualValues(actual), count(3),
        arguments({v1, v2, v3}) {}

  sdsp_nodiscard virtual const char *methodName() const = 0;

  sdsp_nodiscard virtual const char *typeOfTestName() const = 0;

  sdsp_nodiscard virtual const char *getArgumentName(size_t) const {
    return nullptr;
  }

  sdsp_nodiscard virtual T generateValue(const TestInterface &) const = 0;

  void test() const override {
    T expected;
    T actual;
    bool expectedThrown = effectiveValue(expected, expectedValues);
    bool actualThrown = effectiveValue(actual, actualValues);

    bool result =
        isCorrectResult(expected, expectedThrown, actual, actualThrown);
    if (!result) {
      expectedThrown = effectiveValue(expected, expectedValues);
      actualThrown = effectiveValue(actual, actualValues);
      isCorrectResult(expected, expectedThrown, actual, actualThrown);
    }
    BOOST_CHECK_MESSAGE(result, *this);
  }

  std::ostream &print(std::ostream &output) const override {
    T expected;
    T actual;
    bool expectedThrown = effectiveValue(expected, expectedValues);
    bool actualThrown = effectiveValue(actual, actualValues);

    output << typeOfTestName() << "::" << methodName() << "(";
    for (size_t i = 0; i < count; i++) {
      if (i > 0) {
        output << ", ";
      }
      output << getArgumentNameOrDefault(i) << "=" << arguments.at(i);
    }
    output << ")";

    if (isCorrectResult(expected, expectedThrown, actual, actualThrown)) {
      output << ": correct result(";
      printValue(output, expected, expectedThrown);
      return output << ")";
    }
    output << ": expected(";
    printValue(output, expected, expectedThrown);
    output << ") got (";
    printValue(output, actual, actualThrown);
    return output << ")";
  }

  sdsp_nodiscard const char *getArgumentNameOrDefault(size_t i) const {
    const char *string = getArgumentName(i);
    if (string) {
      return string;
    }
    if (i >= count) {
      return "undefined";
    }
    if (count == 1) {
      return "value";
    }
    switch (i) {
    case 0:
      return "v1";
    case 1:
      return "v2";
    case 2:
      return "v3";
    default:
      return "<unknown>";
    }
  }

  bool isCorrectResult(T expected, bool expectedThrown, T actual,
                       bool actualThrown) const {
    return expectedThrown ? actualThrown : !actualThrown && expected == actual;
  };
};
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

std::ostream &operator<<(std::ostream &stream, const AbstractValueTestCase &s) {
  s.print(stream);
  return stream;
}

#pragma clang diagnostic pop

} // namespace

} // namespace simpledsp::testhelper

#endif // SIMPLE_DSP_TEST_HELPER_H
