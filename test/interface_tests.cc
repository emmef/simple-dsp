//
// Created by michel on 05-01-20.
//

#define SIMPLE_DSP_INTERFACE_SIZELIMIT 1024
#include <simple-dsp/interface.h>
#include <boost/test/unit_test.hpp>

using Frequency = u_int32_t;
using Interface = simpledsp::Interface<Frequency>;


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

  BOOST_AUTO_TEST_CASE(ZeroInputsThrows) {
    BOOST_CHECK_THROW(Interface::of(0, outputs, sampleRate, true, bufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(ZeroOutputThrows) {
    BOOST_CHECK_THROW(Interface::of(inputs, 0, sampleRate, true, bufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(ZeroBufferSizeThrows) {
    BOOST_CHECK_THROW(Interface::of(inputs, outputs, sampleRate, true, 0),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(ReasonableSizesThrowsNot) {
    Interface::of(inputs, outputs, sampleRate, true, bufferSize);
  }

  BOOST_AUTO_TEST_CASE(TooLargeInputsThrows) {
    BOOST_CHECK_THROW(Interface::of(largeInputs, outputs, sampleRate, true, bufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(TooLargeOutputsThrows) {
    BOOST_CHECK_THROW(Interface::of(inputs, largeOutputs, sampleRate, true, bufferSize),
            std::invalid_argument );
  }

  BOOST_AUTO_TEST_CASE(TooLargeBufferSizeThrows) {
    BOOST_CHECK_THROW(Interface::of(inputs, outputs, sampleRate, true, largeBufferSize),
            std::invalid_argument );
  }
}
