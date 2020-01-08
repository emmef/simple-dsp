//
// Created by michel on 05-01-20.
//

#define SIMPLE_DSP_INTERFACE_SIZELIMIT 2048
#include <simple-dsp/interface.h>
#include <boost/test/unit_test.hpp>

using Frequency = u_int32_t;
using Interface = simpledsp::Interface<Frequency>;
using Rate = simpledsp::SampleRate<Frequency>;


namespace {
  constexpr size_t limit = SIMPLE_DSP_INTERFACE_SIZELIMIT;
  constexpr size_t inputs = 2;
  constexpr size_t outputs = 2;
  constexpr size_t bufferSize = 256;

  constexpr size_t largeInputs = 1 + limit / (outputs * bufferSize);
  constexpr size_t largeOutputs = 1 + limit / (inputs * bufferSize);
  constexpr size_t largeBufferSize = 1 + limit / (inputs * outputs);
  const simpledsp::SampleRate<Frequency> sampleRate(44100.0f);
}

BOOST_AUTO_TEST_SUITE(InterfaceTest)

  BOOST_AUTO_TEST_CASE(reasonableSizesThrowsNot) {
    Interface::of(inputs, outputs, sampleRate, true, bufferSize);
  }

  BOOST_AUTO_TEST_CASE(zeroInputsThrows) {
    BOOST_CHECK_THROW(Interface::of(0, outputs, sampleRate, true, bufferSize),
            std::invalid_argument );
  }
  BOOST_AUTO_TEST_CASE(withZeroInputsThrows) {
    Interface interface = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    BOOST_CHECK_THROW(interface.withInputs(0),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(zeroOutputThrows) {
    BOOST_CHECK_THROW(Interface::of(inputs, 0, sampleRate, true, bufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(withZeroOutputThrows) {
    Interface interface = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    BOOST_CHECK_THROW(interface.withOutputs(0),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(zeroBufferSizeThrows) {
    BOOST_CHECK_THROW(Interface::of(inputs, outputs, sampleRate, true, 0),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(withZeroBufferSizeThrows) {
    Interface interface = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    BOOST_CHECK_THROW(interface.withBufferSize(0),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(tooLargeInputsThrows) {
    BOOST_CHECK_THROW(Interface::of(largeInputs, outputs, sampleRate, true, bufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(withTooLargeInputsThrows) {
    Interface interface = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    BOOST_CHECK_THROW(interface.withInputs(largeInputs),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(tooLargeOutputsThrows) {
    BOOST_CHECK_THROW(Interface::of(inputs, largeOutputs, sampleRate, true, bufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(withTooLargeOutputsThrows) {
    Interface interface = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    BOOST_CHECK_THROW(interface.withOutputs(largeOutputs),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(tooLargeBufferSizeThrows) {
    BOOST_CHECK_THROW(Interface::of(inputs, outputs, sampleRate, true, largeBufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(withTooLargeBufferSizeThrows) {
    Interface interface = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    BOOST_CHECK_THROW(interface.withBufferSize(largeBufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(sameValuesEqual) {
    Interface interface1 = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    Interface interface2 = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    BOOST_CHECK(interface1 == interface2);
  }

  BOOST_AUTO_TEST_CASE(differentInputsNotEqual) {
    Interface interface1 = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    Interface interface2 = Interface::of(inputs + 1, outputs, sampleRate, true, bufferSize);
    BOOST_CHECK(interface1 != interface2);
  }

  BOOST_AUTO_TEST_CASE(differentOutputsNotEqual) {
    Interface interface1 = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    Interface interface2 = Interface::of(inputs, outputs + 1, sampleRate, true, bufferSize);
    BOOST_CHECK(interface1 != interface2);
  }

  BOOST_AUTO_TEST_CASE(differentSampleRateNotEqual) {
    Interface interface1 = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    Interface interface2 = Interface::of(inputs, outputs, Rate(sampleRate.rate() + 1), true,
            bufferSize);
    BOOST_CHECK(interface1 != interface2);
  }

  BOOST_AUTO_TEST_CASE(differentRealTimeNotEqual) {
    Interface interface1 = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    Interface interface2 = Interface::of(inputs, outputs, sampleRate, false, bufferSize);
    BOOST_CHECK(interface1 != interface2);
  }

  BOOST_AUTO_TEST_CASE(differentBufferSizeNotEqual) {
    Interface interface1 = Interface::of(inputs, outputs, sampleRate, true, bufferSize);
    Interface interface2 = Interface::of(inputs, outputs, sampleRate, false, bufferSize + 1);
    BOOST_CHECK(interface1 != interface2);
  }
}
