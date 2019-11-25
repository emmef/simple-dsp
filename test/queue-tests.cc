//
// Created by michel on 21-11-19.
//

#include <simple-dsp/queue.h>
#include <boost/test/unit_test.hpp>

using Q = simpledsp::Queue<int>;
using R = simpledsp::QueueResult;

namespace {
  constexpr int QUEUESIZE = 9;

  class Generator {
    int read_;
    int write_;
  public:
    Generator() :read_(1), write_(1) { }

    [[nodiscard]] int read() const {
      return read_;
    }

    [[nodiscard]] int write() const {
      return write_;
    }

    int nextRead() {
      return ++read_;
    }

    int nextWrite() {
      return ++write_;
    }
  };

  void fillThenEmptyBufferSize(Q& queue, Generator& generator) {
    int size = queue.capacity();
    for (int i = 0; i < size; i++) {
      BOOST_CHECK_MESSAGE(
              queue.put(generator.nextWrite()) == R::SUCCESS,
              "fillThenEmptyBufferSize: Should be able to put value.");
    }
    for (int i = 0; i < size; i++) {
      int result;
      BOOST_CHECK_MESSAGE(
              queue.get(result) == R::SUCCESS,
              "fillThenEmptyBufferSize: Should be able to get value.");
      BOOST_CHECK_MESSAGE(
              result == generator.nextRead(),
              "fillThenEmptyBufferSize: Get values should correspond with put values.");
    }
  }

  void fillMoreThanBufferSize(Q& queue, Generator& generator) {
    int size = queue.capacity();

    for (int i = 0; i < size; i++) {
      BOOST_CHECK_MESSAGE(
              queue.put(generator.nextWrite()) == R::SUCCESS,
              "fillMoreThanBufferSize: Should be able to put value.");
    }
    BOOST_CHECK_MESSAGE(
            queue.put(generator.nextWrite()) == R::FULL,
            "fillMoreThanBufferSize: Should not be able to put value.");

    for (int i = 0; i < size; i++) {
      int result;
      BOOST_CHECK_MESSAGE(
              queue.get(result) == R::SUCCESS,
              "fillThenEmptyBufferSize: Should be able to get value.");
      BOOST_CHECK_MESSAGE(
              result == generator.nextRead(),
              "fillThenEmptyBufferSize: Get values should correspond with put values.");
    }
  }

  void synchronousPutAndGet(Q& queue, Generator& generator, int size) {
    for (int i = 0; i < size; i++) {
      BOOST_CHECK_MESSAGE(
              queue.put(generator.nextWrite()) == R::SUCCESS,
              "synchronousPutAndGet: Should be able to put value.");
      int result;
      BOOST_CHECK_MESSAGE(
              queue.get(result) == R::SUCCESS,
              "synchronousPutAndGet: Should be able to get value.");
      BOOST_CHECK_MESSAGE(
              result == generator.nextRead(),
              "synchronousPutAndGet: Get values should correspond with put values.");
    }
  }
}

BOOST_AUTO_TEST_SUITE(QueueTests)

  BOOST_AUTO_TEST_CASE(initQueueSize0Throws) {
    std::unique_ptr<Q> x;
    BOOST_CHECK_THROW(x = std::make_unique<Q>(0), std::invalid_argument);
  }

  BOOST_AUTO_TEST_CASE(initQueueTooLargeSizeThrows) {
    size_t size = 1 + simpledsp::queue_data::DefaultData<int>::maxCapacity;
    std::unique_ptr<Q> x;
    BOOST_CHECK_THROW(x = std::make_unique<Q>(size), std::invalid_argument);
  }

  BOOST_AUTO_TEST_CASE(writeUntilFullThenRead) {
    Q queue(QUEUESIZE);
    Generator generator;

    fillThenEmptyBufferSize(queue, generator);
  }

  BOOST_AUTO_TEST_CASE(putAndGetSyncedMoreThanFullSize) {
    Q queue(QUEUESIZE);
    Generator generator;

    synchronousPutAndGet(queue, generator, QUEUESIZE * 2);
  }

  BOOST_AUTO_TEST_CASE(writeSomeEmptyThenwriteUntilFullThenRead) {
    Q queue(QUEUESIZE);
    Generator generator;

    synchronousPutAndGet(queue, generator, QUEUESIZE / 3);
    fillThenEmptyBufferSize(queue, generator);
    fillMoreThanBufferSize(queue, generator);
  }

  BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeInitial) {
    Q queue(QUEUESIZE);
    Generator generator;
    fillMoreThanBufferSize(queue, generator);
  }

  BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeAlreadyUsedSomewhat) {
    Q queue(QUEUESIZE);
    Generator generator;
    synchronousPutAndGet(queue, generator, QUEUESIZE / 3);
    fillMoreThanBufferSize(queue, generator);
  }

  BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize) {
    Q queue(QUEUESIZE);
    Generator generator;
    synchronousPutAndGet(queue, generator, QUEUESIZE * 3);
    fillMoreThanBufferSize(queue, generator);
  }

BOOST_AUTO_TEST_SUITE_END()

