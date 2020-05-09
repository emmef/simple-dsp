//
// Created by michel on 21-11-19.
//

#include <boost/test/unit_test.hpp>
#include <simple-dsp/util/queue.h>
#include <typeinfo>

using Q = simpledsp::Queue<int>;
using R = simpledsp::QueueResult;

namespace {
constexpr int QUEUESIZE = 9;

class Generator {
  int read_;
  int write_;

public:
  Generator() : read_(1), write_(1) {}

  [[nodiscard]] int read() const { return read_; }

  [[nodiscard]] int write() const { return write_; }

  int nextRead() { return ++read_; }

  int nextWrite() { return ++write_; }
};

template <class Q>
void fillThenEmptyBufferSize(Q &queue, Generator &generator) {
  int size = queue.capacity();
  std::string message = "\n\t";
  message += typeid(Q).name();
  message +=
      " of size " + std::to_string(size) + ":\n\t fillThenEmptyBufferSize: ";
  for (int i = 0; i < size; i++) {
    BOOST_CHECK_MESSAGE(queue.put(generator.nextWrite()) == R::SUCCESS,
                        message + "Should be able to put value.");
  }
  for (int i = 0; i < size; i++) {
    int result;
    BOOST_CHECK_MESSAGE(queue.get(result) == R::SUCCESS,
                        message + "Should be able to get value.");
    BOOST_CHECK_MESSAGE(result == generator.nextRead(),
                        message +
                            "Get values should correspond with put values.");
  }
}

template <class Q> void fillMoreThanBufferSize(Q &queue, Generator &generator) {
  int size = queue.capacity();

  std::string message = "\n\t";
  message += typeid(Q).name();
  message +=
      " of size " + std::to_string(size) + ":\n\tfillMoreThanBufferSize: ";
  for (int i = 0; i < size; i++) {
    BOOST_CHECK_MESSAGE(queue.put(generator.nextWrite()) == R::SUCCESS,
                        message + "Should be able to put value.");
  }

  BOOST_CHECK_MESSAGE(queue.put(generator.nextWrite()) == R::FULL,
                      message + "Should not be able to put value.");

  for (int i = 0; i < size; i++) {
    int result;
    BOOST_CHECK_MESSAGE(queue.get(result) == R::SUCCESS,
                        message + "Should be able to get value.");
    BOOST_CHECK_MESSAGE(result == generator.nextRead(),
                        message +
                            "Get values should correspond with put values.");
  }
}

template <class Q>
void synchronousPutAndGet(Q &queue, Generator &generator, int size) {
  std::string message = "\n\t";
  message += typeid(Q).name();
  message += " of size " + std::to_string(queue.capacity()) +
             ":\n\tsynchronousPutAndGet: ";
  for (int i = 0; i < size; i++) {
    BOOST_CHECK_MESSAGE(queue.put(generator.nextWrite()) == R::SUCCESS,
                        message + "Should be able to put value.");
    int result;
    BOOST_CHECK_MESSAGE(queue.get(result) == R::SUCCESS,
                        message + "Should be able to get value.");
    BOOST_CHECK_MESSAGE(result == generator.nextRead(),
                        message +
                            "Get values should correspond with put values.");
  }
}

template <class Q> void writeUntilFullThenRead(size_t queueSize) {
  Q queue(queueSize);
  Generator generator;

  fillThenEmptyBufferSize(queue, generator);
}

template <class Q> void putAndGetSyncedMoreThanFullSize(size_t queueSize) {
  Q queue(queueSize);
  Generator generator;

  synchronousPutAndGet(queue, generator, QUEUESIZE * 2);
}

template <class Q>
void writeSomeEmptyThenwriteUntilFullThenRead(size_t queueSize) {
  Q queue(queueSize);
  Generator generator;

  synchronousPutAndGet(queue, generator, QUEUESIZE / 3);
  fillThenEmptyBufferSize(queue, generator);
  fillMoreThanBufferSize(queue, generator);
}

template <class Q> void writeMoreThanBufferSizeInitial(size_t queueSize) {
  Q queue(queueSize);
  Generator generator;
  fillMoreThanBufferSize(queue, generator);
}

template <class Q>
void writeMoreThanBufferSizeAlreadyUsedSomewhat(size_t queueSize) {
  Q queue(queueSize);
  Generator generator;
  synchronousPutAndGet(queue, generator, QUEUESIZE / 3);
  fillMoreThanBufferSize(queue, generator);
}

template <class Q>
void writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize(size_t queueSize) {
  Q queue(queueSize);
  Generator generator;
  synchronousPutAndGet(queue, generator, QUEUESIZE * 3);
  fillMoreThanBufferSize(queue, generator);
}

} // namespace

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

BOOST_AUTO_TEST_CASE(writeUntilFullThenReadNormalSize) {
  writeUntilFullThenRead<simpledsp::Queue<int>>(QUEUESIZE);
  writeUntilFullThenRead<simpledsp::QueueUnsafe<int>>(QUEUESIZE);
  writeUntilFullThenRead<simpledsp::QueueProducerConsumer<int>>(QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeUntilFullThenReadSizeOne) {
  writeUntilFullThenRead<simpledsp::Queue<int>>(1);
  writeUntilFullThenRead<simpledsp::QueueUnsafe<int>>(1);
  writeUntilFullThenRead<simpledsp::QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(putAndGetSyncedMoreThanFullSizeNormalSize) {
  putAndGetSyncedMoreThanFullSize<simpledsp::Queue<int>>(QUEUESIZE);
  putAndGetSyncedMoreThanFullSize<simpledsp::QueueUnsafe<int>>(QUEUESIZE);
  putAndGetSyncedMoreThanFullSize<simpledsp::QueueProducerConsumer<int>>(
      QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(putAndGetSyncedMoreThanFullSizeSizeOne) {
  putAndGetSyncedMoreThanFullSize<simpledsp::Queue<int>>(QUEUESIZE);
  putAndGetSyncedMoreThanFullSize<simpledsp::QueueUnsafe<int>>(QUEUESIZE);
  putAndGetSyncedMoreThanFullSize<simpledsp::QueueProducerConsumer<int>>(
      QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeSomeEmptyThenwriteUntilFullThenReadNormalSize) {
  writeSomeEmptyThenwriteUntilFullThenRead<simpledsp::Queue<int>>(QUEUESIZE);
  writeSomeEmptyThenwriteUntilFullThenRead<simpledsp::QueueUnsafe<int>>(
      QUEUESIZE);
  writeSomeEmptyThenwriteUntilFullThenRead<
      simpledsp::QueueProducerConsumer<int>>(QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeSomeEmptyThenwriteUntilFullThenReadSizeOne) {
  writeSomeEmptyThenwriteUntilFullThenRead<simpledsp::Queue<int>>(1);
  writeSomeEmptyThenwriteUntilFullThenRead<simpledsp::QueueUnsafe<int>>(1);
  writeSomeEmptyThenwriteUntilFullThenRead<
      simpledsp::QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeInitialNormalSize) {
  writeMoreThanBufferSizeInitial<simpledsp::Queue<int>>(QUEUESIZE);
  writeMoreThanBufferSizeInitial<simpledsp::QueueUnsafe<int>>(QUEUESIZE);
  writeMoreThanBufferSizeInitial<simpledsp::QueueProducerConsumer<int>>(
      QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeInitialSizeOne) {
  writeMoreThanBufferSizeInitial<simpledsp::Queue<int>>(1);
  writeMoreThanBufferSizeInitial<simpledsp::QueueUnsafe<int>>(1);
  writeMoreThanBufferSizeInitial<simpledsp::QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeAlreadyUsedSomewhatNormalSize) {
  writeMoreThanBufferSizeAlreadyUsedSomewhat<simpledsp::Queue<int>>(QUEUESIZE);
  writeMoreThanBufferSizeAlreadyUsedSomewhat<simpledsp::QueueUnsafe<int>>(
      QUEUESIZE);
  writeMoreThanBufferSizeAlreadyUsedSomewhat<
      simpledsp::QueueProducerConsumer<int>>(QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeAlreadyUsedSomewhatSizeOne) {
  writeMoreThanBufferSizeAlreadyUsedSomewhat<simpledsp::Queue<int>>(1);
  writeMoreThanBufferSizeAlreadyUsedSomewhat<simpledsp::QueueUnsafe<int>>(1);
  writeMoreThanBufferSizeAlreadyUsedSomewhat<
      simpledsp::QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(
    writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSizeNormalSize) {
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<simpledsp::Queue<int>>(
      QUEUESIZE);
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<
      simpledsp::QueueUnsafe<int>>(QUEUESIZE);
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<
      simpledsp::QueueProducerConsumer<int>>(QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(
    writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSizeSizeOne) {
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<simpledsp::Queue<int>>(
      1);
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<
      simpledsp::QueueUnsafe<int>>(1);
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<
      simpledsp::QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(testAtomicBehaviourToSeeIfIAmMad) {
  std::atomic<int> atomic = 3;
  int expected = 5;
  BOOST_CHECK_MESSAGE(!atomic.compare_exchange_strong(expected, 6),
                      "Expected compare and exchange failure for "
                      "atomic(3).compare_exchange(5, 6)");
  BOOST_CHECK_MESSAGE(expected == 3, "Expected result in `expected` parameter");
}

BOOST_AUTO_TEST_SUITE_END()
