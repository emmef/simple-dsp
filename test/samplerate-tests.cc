//
// Created by michel on 08-01-20.
//

#include <type_traits>
#include <simple-dsp/samplerate.h>
#include <boost/test/unit_test.hpp>

using Integer = uint64_t ;
using Real = float;
using IntRate = simpledsp::SampleRate<Integer>;
using FloatRate = simpledsp::SampleRate<Real>;
Integer maxInt = std::numeric_limits<Integer>::max();
Integer maxFloatInt = Integer(1) << std::numeric_limits<Real>::digits;

BOOST_AUTO_TEST_SUITE(SampleRateTests)

  BOOST_AUTO_TEST_CASE(sameSampleRateRepresentableInDifferentTypeEqual) {
    Integer rate = 44100;
    IntRate iRate(rate);
    FloatRate fRate(rate);
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
    FloatRate fRate(limit);
    BOOST_CHECK(iRate != fRate);
    BOOST_CHECK(fRate != iRate);
    BOOST_CHECK(limit != fRate);
    BOOST_CHECK(limit == iRate);
  }

  BOOST_AUTO_TEST_CASE(sameSampleRateTooLargeInDifferentTypeNotEqual) {
    Real rate = 1.1 * maxInt;
    IntRate iRate(rate);
    FloatRate fRate(rate);
    BOOST_CHECK(iRate != fRate);
    BOOST_CHECK(fRate != iRate);
    BOOST_CHECK(rate == fRate);
    BOOST_CHECK(rate != iRate);
  }

  BOOST_AUTO_TEST_CASE(sameSampleRateTooSmallInDifferentTypeNotEqual) {
    Real rate = 0.01;
    IntRate iRate(rate);
    FloatRate fRate(rate);
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
    FloatRate fRate(rate);
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
}
