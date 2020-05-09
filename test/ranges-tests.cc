//
// Created by michel on 15-08-19.
//

#include <cstring>
#include <iostream>

#include <simple-dsp/core/bounds.h>
#include <simple-dsp/core/index.h>

#include "test-helper.h"

using namespace std;

namespace {

static constexpr int SIZE_BITS = 10;
static constexpr size_t SIZE_LIMIT = size_t(1) << SIZE_BITS;
using FixedRange = simpledsp::Size<char, size_t, SIZE_BITS>;

using Functions = simpledsp::testhelper::FunctionTestCases;

using TestCase = simpledsp::testhelper::AbstractValueTestCase;

struct WithinTests {
  static TestCase *createWithin(bool expected, size_t value, size_t min,
                                size_t max) {
    return Functions::create("is_within", simpledsp::is_within, expected, value,
                             min, max);
  }
  static TestCase *createWithinExcl(bool expected, size_t value, size_t min,
                                    size_t max) {
    return Functions::create("is_within_excl", simpledsp::is_within_excl,
                             expected, value, min, max);
  }
};

std::vector<simpledsp::testhelper::AbstractValueTestCase *> *
generateTestCases() {
  constexpr size_t MAX_LIMIT = FixedRange::Size::max;

  std::vector<size_t> singleValues;
  singleValues.push_back(0);
  singleValues.push_back(1);
  singleValues.push_back(2);
  singleValues.push_back(3);
  singleValues.push_back(MAX_LIMIT - 1);
  singleValues.push_back(MAX_LIMIT);
  singleValues.push_back(MAX_LIMIT + 1);

  struct Pair {
    size_t v1;
    size_t v2;
  };

  std::vector<Pair> productValues;
  for (size_t i = 0; i <= MAX_LIMIT + 1; i++) {
    for (size_t j = i; j <= MAX_LIMIT + 1; j++) {
      size_t product = j * i;
      if (product < 4 || (product > MAX_LIMIT - 2 && product < MAX_LIMIT + 2)) {
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

  auto testCases =
      new std::vector<simpledsp::testhelper::AbstractValueTestCase *>();

  for (size_t i : singleValues) {
    bool isWithin = i >= 0 && i <= MAX_LIMIT;
    bool isWithinExcl = i > 0 && i < MAX_LIMIT;
    bool isValidSize = i > 0 && i <= MAX_LIMIT;
    bool isValidIndex = i >= 0 && i < MAX_LIMIT;

    testCases->push_back(WithinTests::createWithin(isWithin, i, 0, MAX_LIMIT));
    testCases->push_back(
        WithinTests::createWithinExcl(isWithinExcl, i, 0, MAX_LIMIT));

    testCases->push_back(Functions::create("FixedRange::is_valid",
                                           FixedRange::Size::is_valid,
                                           isValidSize, i));

    testCases->push_back(Functions::create("FixedRange::is_valid_index",
                                           FixedRange::Size::is_valid_index,
                                           isValidIndex, i));
    if (isValidIndex) {
      testCases->push_back(Functions::create("FixedRange::get_valid_index",
                                             FixedRange::get_valid_index,
                                             i, i));
    }
    else {
      testCases->push_back(Functions::create("FixedRange::get_valid_index",
                                             FixedRange::get_valid_index,
                                             i));
    }
    if (isValidSize) {
      testCases->push_back(Functions::create("FixedRange::get_valid_size",
                                             FixedRange::Size::get_valid_size,
                                             i, i));
    }
    else {
      testCases->push_back(Functions::create("FixedRange::get_valid_size",
                                             FixedRange::Size::get_valid_size,
                                             i));

    }
  }

  for (Pair &i : productValues) {
    size_t product = i.v1 * i.v2;
    bool isValidSizeProduct = product > 0 && product <= MAX_LIMIT;

    testCases->push_back(Functions::create("FixedRange::is_valid_product",
                                           FixedRange::is_valid_product,
                                           isValidSizeProduct, i.v1, i.v2));
    if (isValidSizeProduct) {
      testCases->push_back(Functions::create(
          "FixedRange::get_valid_product",
          FixedRange::get_valid_product, product, i.v1, i.v2));
    }
    else {
      testCases->push_back(Functions::create(
          "FixedRange::get_valid_product",
          FixedRange::get_valid_product, i.v1, i.v2));
    }
  }

  for (Pair &i : sumValues) {
    size_t sum = i.v1 * i.v2;
    bool isValidSizeSum = sum > 0 && sum <= MAX_LIMIT;

    testCases->push_back(Functions::create("FixedRange::is_valid_sum",
                                           FixedRange::is_valid_sum,
                                           isValidSizeSum, i.v1, i.v2));
    if (isValidSizeSum) {
      testCases->push_back(Functions::create("FixedRange::get_valid_sum",
                                             FixedRange::get_valid_sum,
                                             sum, i.v1, i.v2));
    }
    else {
      testCases->push_back(Functions::create("FixedRange::get_valid_sum",
                                             FixedRange::get_valid_sum,
                                             i.v1, i.v2));
    }

  }

  return testCases;
} // namespace

class TestGenerator {

  std::vector<simpledsp::testhelper::AbstractValueTestCase *> *testCases;

public:
  TestGenerator() { testCases = generateTestCases(); }

  ~TestGenerator() {
    if (testCases) {
      for (auto testCase : *testCases) {
        delete testCase;
      }
      delete testCases;
      testCases = nullptr;
    }
  }

  sdsp_nodiscard std::vector<simpledsp::testhelper::AbstractValueTestCase *>
  getTestCases() const {
    return *testCases;
  }

} TEST_GENERATOR;
} // namespace

BOOST_AUTO_TEST_SUITE(testRanges)

BOOST_AUTO_TEST_CASE(testConstructorExactMaxSize) {
  FixedRange size(SIZE_LIMIT);
  BOOST_CHECK_EQUAL(size, SIZE_LIMIT);
}

BOOST_AUTO_TEST_CASE(testConstructorValidSize) {
  BOOST_CHECK_EQUAL((size_t)FixedRange(3), 3);
}

BOOST_AUTO_TEST_CASE(testConstructorTooLargeSize) {
  BOOST_CHECK_THROW(FixedRange(SIZE_LIMIT + 1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testConstructorZeroSize) {
  BOOST_CHECK_THROW(FixedRange(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testAdditionValid) {
  size_t v1 = 5;
  size_t v2 = 128;
  size_t sum = v1 + v2;
  FixedRange size = v1;

  BOOST_CHECK_EQUAL(size + v2, sum);
}

BOOST_AUTO_TEST_CASE(testAdditionTooLarge) {
  size_t v1 = 900;
  size_t v2 = 128;
  FixedRange size = v1;
  FixedRange result = 1;

  BOOST_CHECK_THROW(result = size + v2, std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testProductValid) {
  size_t v1 = 5;
  size_t v2 = 128;
  size_t product = v1 * v2;
  FixedRange size = v1;

  BOOST_CHECK_EQUAL(size * v2, product);
}

BOOST_AUTO_TEST_CASE(testProductTooLarge) {
  size_t v1 = 900;
  size_t v2 = 128;
  FixedRange size = v1;
  FixedRange result = 1;

  BOOST_CHECK_THROW(result = size * v2, std::invalid_argument);
}

BOOST_DATA_TEST_CASE(sample, TEST_GENERATOR.getTestCases()) {
  sample->test();
}


BOOST_AUTO_TEST_SUITE_END()
