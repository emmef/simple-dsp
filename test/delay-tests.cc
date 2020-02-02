//
// Created by michel on 21-08-19.
//

#include <simple-dsp/delay.h>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace {
constexpr const size_t SIZE = 16;
simpledsp::CircularMetric metric((size_t)SIZE);
using ReadFirstDelay =
    simpledsp::DelayBasics<simpledsp::DelayAccessType::READ_THEN_WRITE>;
using WriteFirstDelay =
    simpledsp::DelayBasics<simpledsp::DelayAccessType::WRITE_THEN_READ>;

enum class Configuration { WRITE_THEN_DELTA_READ, READ_THEN_DELTA_WRITE };

struct Delay {
  void configure(size_t value, size_t delay_) {
    if (config == Configuration::READ_THEN_DELTA_WRITE) {
      readPtr = value;
      writePtr = setWriteForDelay(delay_);
    } else {
      writePtr = value;
      readPtr = setReadForDelay(delay_);
    }
    delay = delay_;
  }

  void test() {
    int failures = 0;
    int expected = 0;
    int actual = 0;
    for (size_t time = 0; time <= delay + metric.getSize(); time++) {
      int delayed = access(time);
      if (time >= delay) {
        if (delayed != int(time - delay)) {
          if (failures == 0) {
            expected = time - delay;
            actual = delayed;
            std::cout << "FAIL time=" << time << "; write=" << writePtr
                      << "; read=" << readPtr << "; buffer:";
            for (size_t i = 0; i < SIZE; i++) {
              if (i == writePtr) {
                if (i == readPtr) {
                  std::cout << " RW";
                } else {
                  std::cout << "  W";
                }
              } else if (i == readPtr) {
                std::cout << "  R";
              } else {
                std::cout << "   ";
              }

              std::cout << "[" << i << "]=" << act_values[i];
            }
            std::cout << std::endl;
          }
          failures++;
        }
      }
      metric.setNext(writePtr);
      metric.setNext(readPtr);
    }
    if (failures) {
      BOOST_CHECK_EQUAL(expected, actual);
    }
  }

protected:
  virtual size_t setReadForDelay(size_t delay) = 0;
  virtual size_t setWriteForDelay(size_t delay) = 0;
  virtual int access(int newValue) = 0;
  Configuration config;
  size_t readPtr = 0;
  size_t writePtr = 0;
  size_t delay;
  int act_values[SIZE];
};

struct ReadThenWriteDelay : public Delay {
protected:
  size_t setReadForDelay(size_t delay_) override {
    return ReadFirstDelay::getReadPtrForDelay(metric, writePtr, delay_);
  }
  size_t setWriteForDelay(size_t delay_) override {
    return ReadFirstDelay::getWritePtrForDelay(metric, readPtr, delay_);
  };
  int access(int newValue) override {
    return ReadFirstDelay::access(act_values + writePtr, act_values + readPtr,
                                  newValue);
  }
};

struct WriteThenReadDelay : public Delay {
protected:
  size_t setReadForDelay(size_t delay_) override {
    return WriteFirstDelay::getReadPtrForDelay(metric, writePtr, delay_);
  }
  size_t setWriteForDelay(size_t delay_) override {
    return WriteFirstDelay::getWritePtrForDelay(metric, readPtr, delay_);
  };
  int access(int newValue) override {
    return WriteFirstDelay::access(act_values + writePtr, act_values + readPtr,
                                   newValue);
  }
};

struct DelayTestParameters {
  size_t firstValue;
  size_t delay;
  simpledsp::DelayAccessType type;
  Configuration config;

  std::ostream &print(std::ostream &out) const {
    out << (type == simpledsp::DelayAccessType::READ_THEN_WRITE
                ? "ReadFirstDelay"
                : "WriteFirstDelay");
    out << "(";
    out << (config == Configuration::READ_THEN_DELTA_WRITE ? "read="
                                                           : "write=");
    out << firstValue;
    out << "; delay=";
    out << delay;
    out << ")";
    return out;
  }

  void test() const {
    ReadThenWriteDelay rtwDelay;
    WriteThenReadDelay wtrDelay;
    Delay &delayImpl = (type == simpledsp::DelayAccessType::READ_THEN_WRITE)
                           ? *static_cast<Delay *>(&rtwDelay)
                           : *static_cast<Delay *>(&wtrDelay);
    delayImpl.configure(firstValue, delay);
    delayImpl.test();
  }
};

class TestCaseGenerator {
  std::vector<DelayTestParameters> parameters;

public:
  TestCaseGenerator() {
    for (size_t start = 0; start < metric.getSize(); start++) {
      for (Configuration config : {Configuration::READ_THEN_DELTA_WRITE,
                                   Configuration::WRITE_THEN_DELTA_READ}) {
        for (size_t delay = 1; delay <= metric.getSize(); delay++) {
          parameters.push_back({start, delay,
                                simpledsp::DelayAccessType::READ_THEN_WRITE,
                                config});
        }
        for (size_t delay = 0; delay < metric.getSize(); delay++) {
          parameters.push_back({start, delay,
                                simpledsp::DelayAccessType::WRITE_THEN_READ,
                                config});
        }
      }
    }
  }

  const std::vector<DelayTestParameters> testSamples() { return parameters; }
} TEST_GENERATOR;
} // namespace

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
namespace std {
std::ostream &operator<<(std::ostream &stream, const DelayTestParameters &s) {
  s.print(stream);
  return stream;
}
} // namespace std
#pragma clang diagnostic pop

BOOST_AUTO_TEST_SUITE(DelayTests)

// TODO: need many more tests, and useful ones at that...

BOOST_AUTO_TEST_CASE(testReadFirstZeroDelayInvalid) {
  size_t r;
  BOOST_CHECK_THROW(r = ReadFirstDelay::getValidDelay(metric, size_t(0)),
                    std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testWriteFirstZeroDelayIsZero) {
  BOOST_CHECK_EQUAL(WriteFirstDelay::getValidDelay(metric, size_t(0)), 0);
}

BOOST_AUTO_TEST_CASE(testReadFirstMaxDelayIsMax) {
  BOOST_CHECK_EQUAL(ReadFirstDelay::getValidDelay(metric, metric.getSize()),
                    metric.getSize());
}

BOOST_AUTO_TEST_CASE(testWriteFirsMaxDelayIsInvalid) {
  size_t r;
  BOOST_CHECK_THROW(
      r = WriteFirstDelay::getValidDelay(metric, metric.getSize()),
      std::invalid_argument);
}

BOOST_DATA_TEST_CASE(sample, TEST_GENERATOR.testSamples()) { sample.test(); }

BOOST_AUTO_TEST_SUITE_END()
