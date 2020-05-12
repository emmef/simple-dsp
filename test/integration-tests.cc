//
// Created by michel on 10-10-19.
//

#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <iostream>
#include <simple-dsp/integration.h>

using FloatMultipliers = simpledsp::IntegrationMulipliers<float>;
using DoubleMultipliers = simpledsp::IntegrationMulipliers<double>;

#ifdef SIMPLE_DSP_INTEGRATION_LONG_TESTS_SKIP
static constexpr bool skipTests = true;
#else
static constexpr bool skipTests = false;
#endif
template <typename T>
static void impulseResponseSumTest(const char *testName, const char *message,
                                   size_t factor = 1, double epsilon = 1e-6) {
  T output = 0;
  T sum = 0;
  T maxSamples = simpledsp::IntegrationMulipliers<T>::maxSamples() / factor;
  size_t iterationCount = std::sqrt(maxSamples) * 4;
  size_t maxCount = simpledsp::Val::min(static_cast<size_t>(maxSamples),
                             std::numeric_limits<size_t>::max() - 2);

  simpledsp::IntegrationCoefficients<T> integrator(maxSamples);
  auto start = std::chrono::system_clock::now();
  auto maxDuration = std::chrono::seconds(2);

  size_t count = 1;
  integrator.integrate(1.0, output);
  sum = output;
  std::cout << "( " << testName << ": will run for a maximum of "
            << maxDuration.count() << " seconds )" << std::endl;
  for (size_t i = 0; i < iterationCount; i++) {
    for (size_t j = 0; j < iterationCount && count < maxCount; j++, count++) {
      integrator.integrate(0, output);
      sum += output;
    }
    auto now = std::chrono::system_clock::now();
    if (now - start > maxDuration) {
      break;
    }
  }

  T expected = 1 - pow(M_E, -1.0 * count / maxSamples);

  sum /= expected;

  bool result = fabs(1.0 - sum) < epsilon;
  BOOST_CHECK_MESSAGE(result, message);
  if (!result) {
    std::cout << testName << ": " << message << ": " << sum << " for " << count
              << "/" << maxSamples << " samples " << std::endl;
  }
}

template <typename T>
static void stepResponseSumTest(const char *testName, const char *message) {
  T output = 0;
  T maxSamples = simpledsp::IntegrationMulipliers<T>::maxSamples();
  size_t iterationCount = std::sqrt(maxSamples) * 4;
  size_t maxCount = simpledsp::Val::min(static_cast<size_t>(maxSamples),
                             std::numeric_limits<size_t>::max() - 2);

  simpledsp::IntegrationCoefficients<T> integrator(maxSamples);
  auto start = std::chrono::system_clock::now();
  auto maxDuration = std::chrono::seconds(2);
  std::cout << "( " << testName << ": will run for a maximum of "
            << maxDuration.count() << " seconds )" << std::endl;

  size_t count = 1;
  integrator.integrate(1.0, output);
  for (size_t i = 0; i < iterationCount; i++) {
    for (size_t j = 0; j < iterationCount && count < maxCount; j++, count++) {
      integrator.integrate(1, output);
    }
    auto now = std::chrono::system_clock::now();
    if (now - start > maxDuration) {
      break;
    }
  }

  T expected = 1 - pow(M_E, -1.0 * count / maxSamples);

  output /= expected;

  bool result = (1.0 - output) < 0.001;
  BOOST_CHECK_MESSAGE(result, message);
  if (!result) {
    std::cout << testName << ": " << message << ": " << output << " for "
              << count << "/" << maxSamples << " samples " << std::endl;
  }
}

BOOST_AUTO_TEST_SUITE(iirIntegrationTest)

BOOST_AUTO_TEST_CASE(testAndReportSkippingLongTests) {
  if (skipTests) {
    std::cout << "Skipping tests with long duration because "
                 "SIMPLE_DSP_INTEGRATION_LONG_TESTS_SKIP is defined"
              << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(testMaxSamplesFloatOkayBig) {
  BOOST_CHECK_MESSAGE(FloatMultipliers::maxSamples() > 1e6,
                      "Max samples for float integration not reasonable");
}

BOOST_AUTO_TEST_CASE(testFloatImpulseResponseSum) {
  if (skipTests) {
    return;
  }
  impulseResponseSumTest<float>(
      "Integration impulse response sum for single precision floats",
      "sum not within promille of unity", 8, 1e-3);
}

BOOST_AUTO_TEST_CASE(testDoubleImpulseResponseSum) {
  if (skipTests) {
    return;
  }
  impulseResponseSumTest<double>(
      "Integration impulse response sum for double precision floats",
      "sum not within promille of unity");
}

BOOST_AUTO_TEST_CASE(testFloatStepResponseSum) {
  if (skipTests) {
    return;
  }
  stepResponseSumTest<float>(
      "Integration impulse response sum for single precision floats",
      "step response not within promille of unity");
}

BOOST_AUTO_TEST_CASE(testDoubleStepResponseSum) {
  if (skipTests) {
    return;
  }
  stepResponseSumTest<double>(
      "Integration impulse response sum for double precision floats",
      "step response not within promille of unity");
}

BOOST_AUTO_TEST_CASE(testMaxSamplesDoubleOkayBig) {
  BOOST_CHECK_MESSAGE(DoubleMultipliers::maxSamples() > 1e9,
                      "Max samples for double integration not reasonable");
}

BOOST_AUTO_TEST_SUITE_END()
