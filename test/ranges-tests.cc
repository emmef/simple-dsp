//
// Created by michel on 15-08-19.
//

#include <cstring>
#include <iostream>

#include <simple-dsp/core/addressing.h>
#include <simple-dsp/core/algorithm.h>

#include "test-helper.h"

using namespace std;

namespace {

using FixedRange = simpledsp::addr::Elements<char, size_t, 1024>;
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

    testCases->push_back(Functions::create("FixedRange::Size::is_valid",
                                           FixedRange::Size::is_valid,
                                           isValidSize, i));
    if (isValidIndex) {
      testCases->push_back(Functions::create("FixedRange::Index::is_valid",
                                             FixedRange::Index::get_value_if_valid,
                                             i, i, ""));
    }
    else {
      testCases->push_back(Functions::create("FixedRange::Index::is_valid",
                                             FixedRange::Index::get_value_if_valid,
                                             i, ""));
    }
    if (isValidSize) {
      testCases->push_back(Functions::create("FixedRange::Size::is_valid",
                                             FixedRange::Size::get_value_if_valid,
                                             i, i, ""));
    }
    else {
      testCases->push_back(Functions::create("FixedRange::Size::is_valid",
                                             FixedRange::Size::get_value_if_valid,
                                             i, ""));

    }
  }

  for (Pair &i : productValues) {
    size_t product = i.v1 * i.v2;
    bool isValidSizeProduct = product > 0 && product <= MAX_LIMIT;

    testCases->push_back(Functions::create("FixedRange::Size::is_valid_product",
                                           FixedRange::Size::is_valid_product,
                                           isValidSizeProduct, i.v1, i.v2));
    if (isValidSizeProduct) {
      testCases->push_back(Functions::create(
          "FixedRange::Size::get_product_if_valid",
          FixedRange::Size::get_product_if_valid, product, i.v1, i.v2, ""));
      testCases->push_back(Functions::create(
          "FixedRange::Size::get_value_if_valid_product",
          FixedRange::Size::get_value_if_valid_product, i.v1, i.v1, i.v2, ""));
    }
    else {
      testCases->push_back(Functions::create(
          "FixedRange::Size::get_product_if_valid",
          FixedRange::Size::get_product_if_valid, i.v1, i.v2, ""));
      testCases->push_back(Functions::create(
          "FixedRange::Size::get_value_if_valid_product",
          FixedRange::Size::get_value_if_valid_product, i.v1, i.v2, ""));
    }
  }

  for (Pair &i : sumValues) {
    size_t sum = i.v1 * i.v2;
    bool isValidSizeSum = sum > 0 && sum <= MAX_LIMIT;
    bool isValidIndexSum = sum < MAX_LIMIT;

    testCases->push_back(Functions::create("FixedRange::Size::is_valid_sum",
                                           FixedRange::Size::is_valid_sum,
                                           isValidSizeSum, i.v1, i.v2));
    testCases->push_back(Functions::create("FixedRange::Index::is_valid_sum",
                                           FixedRange::Index::is_valid_sum,
                                           isValidIndexSum, i.v1, i.v2));
    if (isValidSizeSum) {
      testCases->push_back(Functions::create("FixedRange::Size::get_sum_if_valid",
                                             FixedRange::Size::get_sum_if_valid,
                                             sum, i.v1, i.v2, ""));
      testCases->push_back(Functions::create(
          "FixedRange::Size::get_value_if_valid_sum",
          FixedRange::Size::get_value_if_valid_sum, i.v1, i.v1, i.v2, ""));
    }
    else {
      testCases->push_back(Functions::create("FixedRange::Size::get_sum_if_valid",
                                             FixedRange::Size::get_sum_if_valid,
                                             i.v1, i.v2, ""));
      testCases->push_back(Functions::create(
          "FixedRange::Size::get_value_if_valid_sum",
          FixedRange::Size::get_value_if_valid_sum, i.v1, i.v2, ""));
    }

    if (isValidIndexSum) {
      testCases->push_back(Functions::create(
          "FixedRange::Index::get_sum_if_valid",
          FixedRange::Index::get_sum_if_valid, sum, i.v1, i.v2, ""));

      testCases->push_back(Functions::create(
          "FixedRange::Index::get_value_if_valid_sum",
          FixedRange::Index::get_value_if_valid_sum, i.v1, i.v1, i.v2, ""));
    }
    else {
      testCases->push_back(Functions::create("FixedRange::Index::get_sum_if_valid",
                                             FixedRange::Index::get_sum_if_valid,
                                             i.v1, i.v2, ""));
      testCases->push_back(Functions::create(
          "FixedRange::Index::get_value_if_valid_sum",
          FixedRange::Index::get_value_if_valid_sum, i.v1, i.v2, ""));
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

    BOOST_DATA_TEST_CASE(sample, TEST_GENERATOR.getTestCases()) {
  sample->test();
}

BOOST_AUTO_TEST_SUITE_END()
