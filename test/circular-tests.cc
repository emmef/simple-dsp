//
// Created by michel on 19-08-19.
//

#include <simple-dsp/circular.h>

#include <boost/test/unit_test.hpp>

namespace {
    using Arithmic = simpledsp::CircularArithmic;
    using Metric = simpledsp::CircularMetricBase<size_t>;

    constexpr size_t requestedSize = 13;
    constexpr size_t properSize = 16;
    constexpr size_t properMask = 15;

    Metric metric(requestedSize);
}

BOOST_AUTO_TEST_SUITE(circularMetricTests)

    BOOST_AUTO_TEST_CASE(testProperSize)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::properCircularSize(requestedSize) == properSize,
                "Proper circular size for 13 is 16");
    }

    BOOST_AUTO_TEST_CASE(testProperMask)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::properCircularMask(requestedSize) == properMask,
                "Proper circular mask for 13 is 15");
    }


    BOOST_AUTO_TEST_CASE(testAddNoWrap)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7, 5, properMask) == 12,
                "Circular(16)::add(7, 5) == 12");
    }

    BOOST_AUTO_TEST_CASE(testLargeAddNoWrap)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7 + properSize, 5, properMask) == 12,
                "Circular(16)::add(7 + size, 5) == 12");
    }

    BOOST_AUTO_TEST_CASE(testLargeAddLargeNoWrap)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7 + properSize, 5 + properSize, properMask) == 12,
                "Circular(16)::add(7 + size, 5 + size) == 12");
    }

    BOOST_AUTO_TEST_CASE(testAddWrapZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7,9 , properMask) == 0,
                "Circular(16)::add(7, 9) == 0");
    }

    BOOST_AUTO_TEST_CASE(testLargeAddWrapZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7 + properSize,9 , properMask) == 0,
                "Circular(16)::add(7 + size, 9) == 0");
    }

    BOOST_AUTO_TEST_CASE(testLargeAddLargeWrapZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7 + properSize, 9 + properSize, properMask) == 0,
                "Circular(16)::add(7 + size, 9 + size) == 0");
    }

    BOOST_AUTO_TEST_CASE(testAddWrapOne)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7,10 , properMask) == 1,
                "Circular(16)::add(7, 10) == 1");
    }

    BOOST_AUTO_TEST_CASE(testLargeAddWrapOne)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7 + properSize,10, properMask) == 1,
                "Circular(16)::add(7 + size, 10) == 1");
    }

    BOOST_AUTO_TEST_CASE(testLargeAddLargeWrapOne)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(7 + properSize,10 + properSize, properMask) == 1,
                "Circular(16)::add(7 + size, 10) == 1");
    }

    BOOST_AUTO_TEST_CASE(testAddNoWrapFromZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(0, 5, properMask) == 5,
                "Circular(16)::add(0, 5) == 5");
    }

    BOOST_AUTO_TEST_CASE(testLargeAddNoWrapFromZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(0 + properSize, 5, properMask) == 5,
                "Circular(16)::add(size, 5) == 5");
    }

    BOOST_AUTO_TEST_CASE(testLargeAddLargeNoWrapFromZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::add(0 + properSize, 5 + properSize, properMask) == 5,
                "Circular(16)::add(size, 5 + size) == 5");
    }


    BOOST_AUTO_TEST_CASE(testSubtractNoWrap)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7, 5, properMask) == 2,
                "Circular(16)::subtract(7, 5) == 2");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractNoWrap)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7 + properSize, 5, properMask) == 2,
                "Circular(16)::subtract(7 + size, 5) == 2");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractLargeNoWrap)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7 + properSize, 5 + properSize, properMask) == 2,
                "Circular(16)::subtract(7 + size, 5 + size) == 2");
    }

    BOOST_AUTO_TEST_CASE(testSubtractNoWrapZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7,7 , properMask) == 0,
                "Circular(16)::subtract(7, 7) == 0");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractNoWrapZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7 + properSize,7 , properMask) == 0,
                "Circular(16)::subtract(7 + size, 7) == 0");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractLargeNoWrapZero)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7 + properSize, 7 + properSize, properMask) == 0,
                "Circular(16)::subtract(7 + size, 7 + size) == 0");
    }

    BOOST_AUTO_TEST_CASE(testSubtractWrapOne)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7,8 , properMask) == properMask,
                "Circular(16)::subtract(7, 8) == mask");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractWrapOne)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7 + properSize,8, properMask) == properMask,
                "Circular(16)::subtract(7 + size, 8) == mask");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractLargeWrapOne)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7 + properSize,8 + properSize, properMask) == properMask,
                "Circular(16)::subtract(7 + size, 8 + size) == mask");
    }

    BOOST_AUTO_TEST_CASE(testSubtractWrapTwo)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7,9 , properMask) == properMask - 1,
                "Circular(16)::subtract(7, 9) == mask - 1");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractWrapTwo)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7 + properSize,9, properMask) == properMask - 1,
                "Circular(16)::subtract(7 + size, 9) == mask - 1");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractLargeWrapTwo)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(7 + properSize,9 + properSize, properMask) == properMask - 1,
                "Circular(16)::subtract(7 + size, 9 + size) == mask - 1");
    }

    BOOST_AUTO_TEST_CASE(testSubtractNoWrapFromMask)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(properMask, 5, properMask) == 10,
                "Circular(16)::subtract(mask, 5) == 10");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractNoWrapFromMask)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(properMask + properSize, 5, properMask) == 10,
                "Circular(16)::subtract(mask + size, 5) == 10");
    }

    BOOST_AUTO_TEST_CASE(testLargeSubtractLargeNoWrapFromMask)
    {
        BOOST_CHECK_MESSAGE(
                Arithmic::subtract(properMask + properSize, 5 + properSize, properMask) == 10,
                "Circular(16)::subtract(mask + size, 5 + size) == 10");
    }

    BOOST_AUTO_TEST_CASE(testRoundtripWithSetNext) {
        size_t reference = 0;
        size_t actual = 0;
        for (size_t i = 0; i <= properSize; i++) {
            reference = (reference + 1) % properSize;
            Arithmic::setNext(actual, properMask);
            BOOST_CHECK_EQUAL(reference, actual);
        }
    }

    BOOST_AUTO_TEST_CASE(testRoundtripWithNext)
    {
        size_t reference = 0;
        size_t actual = 0;
        for (size_t i = 0; i <= properSize; i++) {
            reference = (reference + 1) % properSize;
            actual = Arithmic::next(actual, properMask);
            BOOST_CHECK_EQUAL(reference, actual);
        }
    }

    BOOST_AUTO_TEST_CASE(testRoundtripWithSetPrevious)
    {
        size_t reference = 0;
        size_t actual = 0;
        for (size_t i = 0; i <= properSize; i++) {
            reference = (reference > 0) ? reference - 1 : properMask;
            Arithmic::setPrevious(actual, properMask);
            BOOST_CHECK_EQUAL(reference, actual);
        }
    }

    BOOST_AUTO_TEST_CASE(testRoundtripWithPrevious)
    {
        size_t reference = 0;
        size_t actual = 0;
        for (size_t i = 0; i <= properSize; i++) {
            reference = (reference > 0) ? reference - 1 : properMask;
            actual = Arithmic::previous(actual, properMask);
            BOOST_CHECK_EQUAL(reference, actual);
        }
    }

    BOOST_AUTO_TEST_CASE(testMetricSetSizeOneSmaller)
    {
        Metric metric(requestedSize);
        auto oldSize = metric.getSize();
        if (metric.setSize(properSize - 1)) {
            BOOST_CHECK_MESSAGE(metric.getSize() ==oldSize, "Setting size one below current size yields same size");
        }
    }

    BOOST_AUTO_TEST_CASE(testMetricSetSizeSame)
    {
        Metric metric(requestedSize);
        auto oldSize = metric.getSize();
        if (metric.setSize(properSize)) {
            BOOST_CHECK_MESSAGE(metric.getSize() ==oldSize, "Setting size to same value should not change anything");
        }
    }

    BOOST_AUTO_TEST_CASE(testMetricSetSizeOneBigger)
    {
        Metric metric(requestedSize);
        auto oldSize = metric.getSize();
        if (metric.setSize(properSize + 1)) {
            BOOST_CHECK_MESSAGE(metric.getSize() == 2 * oldSize, "Setting size one above yields twice as big size");
        }
    }

    BOOST_AUTO_TEST_CASE(testMetricSetSizeHalf)
    {
        Metric metric(requestedSize);
        auto oldSize = metric.getSize();
        if (metric.setSize(properSize / 2)) {
            BOOST_CHECK_MESSAGE(metric.getSize() == oldSize / 2, "Setting size to half, yields half size");
        }
    }



BOOST_AUTO_TEST_SUITE_END()
