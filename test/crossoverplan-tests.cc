//
// Created by michel on 29-10-19.
//

#include <iostream>
#include <simple-dsp/crossoverplan.h>
#include <boost/test/unit_test.hpp>

using Step = simpledsp::CrossoverPlan::Step;
using Planner = simpledsp::CrossoverPlan;

namespace {
  bool operator == (
          const Step &step1, const Step &step2) {
    return
            step1.filter() == step2.filter() &&
                    step1.input() == step2.input() &&
                    step1.lowOut() == step2.lowOut() &&
                    step1.highOut() == step2.highOut();
  }

  std::ostream &operator << (std::ostream &output, const Step &step) {
    output <<
           "(on " << step.input() <<
           " apply " << step.filter() <<
           " lowOut->" << step.lowOut() <<
           "; highOut->" << step.highOut() << ")";
    return output;
  }

}

BOOST_AUTO_TEST_SUITE(CrossoverPlanTests)

  void testSameSteps(const std::vector<Step> &expected, const std::vector<Step> &actual) {
    if (expected.size() != actual.size()) {
      BOOST_CHECK_MESSAGE(false, "Size of crossover plans not the same");
      std::cout <<
              "\t\tSize: expected=" << expected.size() <<
              "; actual=" << actual.size() << std::endl;
      return;
    }
    bool same = true;
    for (size_t i = 0; same && i < expected.size(); i++) {
      same = expected[i] == actual[i];
    }
    if (same) {
      return;
    }
    BOOST_CHECK_MESSAGE(false, "Actual step plan not same as expected plan");
    for (size_t i = 0; i < expected.size(); i++) {
      if (expected[i] == actual[i]) {
        std::cout << "\t\t  expected:" << expected[i] << " == actual:" << actual[i] << std::endl;
      } else {
        std::cout << "\t\t* expected:" << expected[i] << " != actual:" << actual[i] << std::endl;
      }
    }
  }

  BOOST_AUTO_TEST_CASE(testSameStepsAreEqual) {
    /*
     * This actually tests the equal operator that is necessary for further testing
     */
    Step step1(1,2,3,4);
    Step step2(1,2,3,4);

    BOOST_CHECK_MESSAGE(
            step1 == step2,
            "Same CrossoverProcessingSteps are also equal (1,2,3,4)");

    step1 = {2,3,5,7};
    step2 = {2,3,5,7};

    BOOST_CHECK_MESSAGE(
            step1 == step2,
            "Same CrossoverProcessingSteps are also equal (2,3,5,7)");
  }

  BOOST_AUTO_TEST_CASE(testDifferentStepsAreNotEqualFactor) {
    /*
     * This actually tests the equal operator that is necessary for further testing
     */

    Step values(1,2,3,4);

    for (int i = 1; i <= 4; i++) {
      Step step1 = values;
      Step step2;
      switch (i) {
      case 1:
        step2 = {step1.filter() * 7, step1.input(), step1.lowOut(), step1.highOut()};
        break;
      case 2:
        step2 = {step1.filter(), step1.input() * 7, step1.lowOut(), step1.highOut()};
        break;
      case 3:
        step2 = {step1.filter(), step1.input(), step1.lowOut() * 7, step1.highOut()};
        break;
      case 4:
        step2 = {step1.filter(), step1.input(), step1.lowOut(), step1.highOut() * 7};
        break;
      default:
        step2 = {step1.filter(), step1.input(), step1.lowOut(), step1.highOut()};
        break;
      }
      bool p = !(step1 == step2);
      if (!p) {
        BOOST_CHECK_MESSAGE(
                p,
                "Different CrossoverProcessingSteps are not equal");
        std::cout << "\t\t" << step1 << " != " << step2 << std::endl;
      }
    }
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSize1) {
    std::vector<Step> plan;
    Planner::create(plan, 1);
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSizeZeroThrowsInvalidArgument) {
    std::vector<Step> plan;
    BOOST_CHECK_THROW(Planner::create(plan, 0), std::invalid_argument);
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSizeOne) {
    std::vector<Step> expected;
    expected.push_back({1, 0, 0, 1});

    std::vector<Step> actual;
    Planner::create(actual, 1);

    testSameSteps(expected, actual);
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSizeTwo) {
    std::vector<Step> expected;
    expected.push_back({2, 1, 1, 2});
    expected.push_back({1, 0, 0, 1});

    std::vector<Step> actual;
    Planner::create(actual, 2);

    testSameSteps(expected, actual);
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSizeThree) {
    std::vector<Step> expected;
    expected.push_back({2, 1, 1, 3});
    expected.push_back({3, 2, 2, 3});
    expected.push_back({1, 0, 0, 1});

    std::vector<Step> actual;
    Planner::create(actual, 3);

    testSameSteps(expected, actual);
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSizeFour) {
    std::vector<Step> expected;
    expected.push_back({3, 2, 2, 4});
    expected.push_back({4, 3, 3, 4});
    expected.push_back({2, 1, 1, 2});
    expected.push_back({1, 0, 0, 1});

    std::vector<Step> actual;
    Planner::create(actual, 4);

    testSameSteps(expected, actual);
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSizeFive) {
    std::vector<Step> expected;
    expected.push_back({3, 2, 2, 5});
    expected.push_back({5, 4, 4, 5});
    expected.push_back({4, 3, 3, 4});
    expected.push_back({2, 1, 1, 2});
    expected.push_back({1, 0, 0, 1});

    std::vector<Step> actual;
    Planner::create(actual, 5);

    testSameSteps(expected, actual);
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSizeSix) {
    std::vector<Step> expected;
    expected.push_back({4, 3, 2, 6});
    expected.push_back({6, 5, 5, 6});
    expected.push_back({5, 4, 4, 5});
    expected.push_back({2, 1, 1, 3});
    expected.push_back({3, 2, 2, 3});
    expected.push_back({1, 0, 0, 1});

    std::vector<Step> actual;
    Planner::create(actual, 6);

    testSameSteps(expected, actual);
  }

  BOOST_AUTO_TEST_CASE(testPlanOfSizeSeven) {
    std::vector<Step> expected;
    expected.push_back({4, 3, 2, 6});
    expected.push_back({6, 5, 5, 7});
    expected.push_back({7, 6, 6, 7});
    expected.push_back({5, 4, 4, 5});
    expected.push_back({2, 1, 1, 3});
    expected.push_back({3, 2, 2, 3});
    expected.push_back({1, 0, 0, 1});

    std::vector<Step> actual;
    Planner::create(actual, 7);

    testSameSteps(expected, actual);
  }

BOOST_AUTO_TEST_SUITE_END()
