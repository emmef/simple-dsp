//
// Created by michel on 15-08-19.
//

#include <cstring>

#include <simple-dsp/algorithm/size.h>

#include <boost/test/unit_test.hpp>

namespace {
    using TestType = double;
    simpledsp::algorithm::SizeLimits<TestType > limits;
}


BOOST_AUTO_TEST_SUITE(SizeAndOffsetLimits)

    // TODO: need many more tests

    BOOST_AUTO_TEST_CASE(testSizeLimitsZero)
    {
        BOOST_CHECK_MESSAGE(limits.validProduct(1, 1) == 1, "One times one is one");
    }

    BOOST_AUTO_TEST_CASE(testSizeLimitsZeroOkay)
    {
        BOOST_CHECK_MESSAGE(limits.validProduct(1, 1) == 1, "One times one is one");
    }

BOOST_AUTO_TEST_SUITE_END()
