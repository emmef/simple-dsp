//
// Created by michel on 21-11-19.
//

#include <boost/test/unit_test.hpp>
#include <simple-dsp/util/queue.h>
#include <typeinfo>

using IntQueue = simpledsp::util::Queue<int>;
using Result = simpledsp::util::QueueResult;

template<typename T>
using Queue = simpledsp::util::Queue<T>;
template<typename T>
using QueueUnsafe = simpledsp::util::QueueUnsafe<T>;
template<typename T>
using QueueProducerConsumer = simpledsp::util::QueueProducerConsumer<T>;

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
    BOOST_CHECK_MESSAGE(queue.put(generator.nextWrite()) == Result::SUCCESS,
                        message + "Should be able to put value.");
  }
  for (int i = 0; i < size; i++) {
    int result;
    BOOST_CHECK_MESSAGE(queue.get(result) == Result::SUCCESS,
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
    BOOST_CHECK_MESSAGE(queue.put(generator.nextWrite()) == Result::SUCCESS,
                        message + "Should be able to put value.");
  }

  BOOST_CHECK_MESSAGE(queue.put(generator.nextWrite()) == Result::FULL,
                      message + "Should not be able to put value.");

  for (int i = 0; i < size; i++) {
    int result;
    BOOST_CHECK_MESSAGE(queue.get(result) == Result::SUCCESS,
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
    BOOST_CHECK_MESSAGE(queue.put(generator.nextWrite()) == Result::SUCCESS,
                        message + "Should be able to put value.");
    int result;
    BOOST_CHECK_MESSAGE(queue.get(result) == Result::SUCCESS,
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
  std::unique_ptr<IntQueue> x;
  BOOST_CHECK_THROW(x = std::make_unique<IntQueue>(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initQueueTooLargeSizeThrows) {
  size_t size = 1 + simpledsp::util::queue_data::DefaultData<int>::maxCapacity;
  std::unique_ptr<IntQueue> x;
  BOOST_CHECK_THROW(x = std::make_unique<IntQueue>(size), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(writeUntilFullThenReadNormalSize) {
  writeUntilFullThenRead<Queue<int>>(QUEUESIZE);
  writeUntilFullThenRead<QueueUnsafe<int>>(QUEUESIZE);
  writeUntilFullThenRead<QueueProducerConsumer<int>>(QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeUntilFullThenReadSizeOne) {
  writeUntilFullThenRead<Queue<int>>(1);
  writeUntilFullThenRead<QueueUnsafe<int>>(1);
  writeUntilFullThenRead<QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(putAndGetSyncedMoreThanFullSizeNormalSize) {
  putAndGetSyncedMoreThanFullSize<Queue<int>>(QUEUESIZE);
  putAndGetSyncedMoreThanFullSize<QueueUnsafe<int>>(QUEUESIZE);
  putAndGetSyncedMoreThanFullSize<QueueProducerConsumer<int>>(
      QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(putAndGetSyncedMoreThanFullSizeSizeOne) {
  putAndGetSyncedMoreThanFullSize<Queue<int>>(QUEUESIZE);
  putAndGetSyncedMoreThanFullSize<QueueUnsafe<int>>(QUEUESIZE);
  putAndGetSyncedMoreThanFullSize<QueueProducerConsumer<int>>(
      QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeSomeEmptyThenwriteUntilFullThenReadNormalSize) {
  writeSomeEmptyThenwriteUntilFullThenRead<Queue<int>>(QUEUESIZE);
  writeSomeEmptyThenwriteUntilFullThenRead<QueueUnsafe<int>>(
      QUEUESIZE);
  writeSomeEmptyThenwriteUntilFullThenRead<
      QueueProducerConsumer<int>>(QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeSomeEmptyThenwriteUntilFullThenReadSizeOne) {
  writeSomeEmptyThenwriteUntilFullThenRead<Queue<int>>(1);
  writeSomeEmptyThenwriteUntilFullThenRead<QueueUnsafe<int>>(1);
  writeSomeEmptyThenwriteUntilFullThenRead<
      QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeInitialNormalSize) {
  writeMoreThanBufferSizeInitial<Queue<int>>(QUEUESIZE);
  writeMoreThanBufferSizeInitial<QueueUnsafe<int>>(QUEUESIZE);
  writeMoreThanBufferSizeInitial<QueueProducerConsumer<int>>(
      QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeInitialSizeOne) {
  writeMoreThanBufferSizeInitial<Queue<int>>(1);
  writeMoreThanBufferSizeInitial<QueueUnsafe<int>>(1);
  writeMoreThanBufferSizeInitial<QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeAlreadyUsedSomewhatNormalSize) {
  writeMoreThanBufferSizeAlreadyUsedSomewhat<Queue<int>>(QUEUESIZE);
  writeMoreThanBufferSizeAlreadyUsedSomewhat<QueueUnsafe<int>>(
      QUEUESIZE);
  writeMoreThanBufferSizeAlreadyUsedSomewhat<
      QueueProducerConsumer<int>>(QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(writeMoreThanBufferSizeAlreadyUsedSomewhatSizeOne) {
  writeMoreThanBufferSizeAlreadyUsedSomewhat<Queue<int>>(1);
  writeMoreThanBufferSizeAlreadyUsedSomewhat<QueueUnsafe<int>>(1);
  writeMoreThanBufferSizeAlreadyUsedSomewhat<
      QueueProducerConsumer<int>>(1);
}

BOOST_AUTO_TEST_CASE(
    writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSizeNormalSize) {
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<Queue<int>>(
      QUEUESIZE);
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<
      QueueUnsafe<int>>(QUEUESIZE);
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<
      QueueProducerConsumer<int>>(QUEUESIZE);
}

BOOST_AUTO_TEST_CASE(
    writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSizeSizeOne) {
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<Queue<int>>(
      1);
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<
      QueueUnsafe<int>>(1);
  writeMoreThanBufferSizeAlreadyUsedMoreThanBufferSize<
      QueueProducerConsumer<int>>(1);
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
