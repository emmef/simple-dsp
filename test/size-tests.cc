//
// Created by michel on 15-08-19.
//

#include <cstring>

#include <simple-dsp/addressing.h>

#include <boost/test/unit_test.hpp>

namespace {
    using TestType = double;
    simpledsp::Size<TestType > sizeLimits;
    simpledsp::Offset<TestType > offsetLimits;
}


BOOST_AUTO_TEST_SUITE(SizeAndOffsetLimits)

    // TODO: need many more tests

    BOOST_AUTO_TEST_CASE(testSizeLimitsZero)
    {
        BOOST_CHECK_MESSAGE(sizeLimits.validProductGet(1, 1, simpledsp::ValidGet::RESULT) == 1, "One times one is one");
    }

    BOOST_AUTO_TEST_CASE(testSizeLimitsZeroOkay)
    {
        BOOST_CHECK_MESSAGE(offsetLimits.validProductGet(1, 1, simpledsp::ValidGet::RESULT) == 1, "One times one is one");
    }

BOOST_AUTO_TEST_SUITE_END()
