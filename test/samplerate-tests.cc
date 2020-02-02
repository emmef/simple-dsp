//
// Created by michel on 08-01-20.
//

#include <boost/test/unit_test.hpp>
#include <simple-dsp/samplerate.h>
#include <type_traits>

using Integer = uint64_t;
using Real = float;
using IntRate = simpledsp::SampleRateBase<Integer>;
using RealRate = simpledsp::SampleRateBase<Real>;
Integer maxInt = std::numeric_limits<Integer>::max();
Integer maxFloatInt = Integer(1) << std::numeric_limits<Real>::digits;

BOOST_AUTO_TEST_SUITE(SampleRateTests)

BOOST_AUTO_TEST_CASE(sameSampleRateRepresentableInDifferentTypeEqual) {
  Integer rate = 44100;
  IntRate iRate(rate);
  RealRate fRate(rate);
  BOOST_CHECK(iRate == fRate);
  BOOST_CHECK(fRate == iRate);
  BOOST_CHECK(rate == iRate);
  BOOST_CHECK(rate == fRate);
}

BOOST_AUTO_TEST_CASE(sameSampleRatePrecisionLossInDifferentTypeNotEqual) {
  Integer limit = maxFloatInt;
  while (Integer(Real(limit)) == limit) {
    limit++;
  }
  IntRate iRate(limit);
  RealRate fRate(limit);
  BOOST_CHECK(iRate != fRate);
  BOOST_CHECK(fRate != iRate);
  BOOST_CHECK(limit != fRate);
  BOOST_CHECK(limit == iRate);
}

BOOST_AUTO_TEST_CASE(sameSampleRateTooLargeInDifferentTypeNotEqual) {
  Real rate = 1.1 * maxInt;
  IntRate iRate(rate);
  RealRate fRate(rate);
  BOOST_CHECK(iRate != fRate);
  BOOST_CHECK(fRate != iRate);
  BOOST_CHECK(rate == fRate);
  BOOST_CHECK(rate != iRate);
}

BOOST_AUTO_TEST_CASE(sameSampleRateTooSmallInDifferentTypeNotEqual) {
  Real rate = 0.01;
  IntRate iRate(rate);
  RealRate fRate(rate);
  BOOST_CHECK(iRate != fRate);
  BOOST_CHECK(fRate != iRate);
  BOOST_CHECK(rate == fRate);
  BOOST_CHECK(rate != iRate);
}

BOOST_AUTO_TEST_CASE(tooSmallIntegerIsClampedToTwo) {
  Real rate = 0.01;
  IntRate iRate(rate);
  BOOST_CHECK(iRate.rate() == 2);
}

BOOST_AUTO_TEST_CASE(tooSmallRealIsClampedToTwoEpsilon) {
  Real rate = std::numeric_limits<Real>::min() / 5;
  RealRate fRate(rate);
  Real actual = fRate.rate();
  Real expected = std::numeric_limits<Real>::min() * 2;
  BOOST_CHECK(actual == expected);
}

BOOST_AUTO_TEST_CASE(tooLargeIsClampedToMaximum) {
  Real rate = std::numeric_limits<Integer>::max() * 3;
  IntRate fRate(rate);
  Integer actual = fRate.rate();
  Integer expected = std::numeric_limits<Integer>::max();
  BOOST_CHECK(actual == expected);
}

BOOST_AUTO_TEST_CASE(relativeRepresentationTest1) {
  Real fourAndAHalf = 4.5;
  // Truncated to 4, which means 1/9 of error
  BOOST_CHECK(IntRate::representationError(fourAndAHalf) == 0.5 / 4.5);
}

BOOST_AUTO_TEST_CASE(relativeRepresentationTest2) {
  Real fourAndAHalf = 4.4;
  BOOST_CHECK(RealRate::representationError(fourAndAHalf) == 0);
}

BOOST_AUTO_TEST_CASE(relativeRepresentationTest3) {
  Integer limit = maxFloatInt;
  while (Integer(Real(limit)) == limit) {
    limit++;
  }
  Real diff = limit - maxFloatInt;
  double realPresentationError = RealRate::representationError(limit);
  BOOST_CHECK(realPresentationError > 0 &&
              realPresentationError <= diff / maxFloatInt);
}
}
