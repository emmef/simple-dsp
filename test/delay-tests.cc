//
// Created by michel on 21-08-19.
//

#include <simple-dsp/delay.h>

#include <boost/test/unit_test.hpp>

namespace {
    simpledsp::VectorDelayOffsetsContainerBase<size_t, simpledsp::DelayAccessType::WRITE_THEN_READ> delays(
            23, 34);

}

BOOST_AUTO_TEST_SUITE(DelayTests)

// TODO: need many more tests, and useful ones at that...

BOOST_AUTO_TEST_CASE(testMetricRoundedUp)
{
    BOOST_CHECK(delays.getMetric().getSize() == 32);
}

BOOST_AUTO_TEST_CASE(testSizeLimitsZeroOkay)
{
    BOOST_CHECK(delays.ref(0) + 1 == delays.ref(1));
}

BOOST_AUTO_TEST_SUITE_END()
