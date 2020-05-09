//
// Created by michel on 27-11-19.
//

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <simple-dsp/util/lock-fee-owner.hpp>

using IntOwner = simpledsp::util::LockfreeOwner<int>;
using Result = simpledsp::util::LockFeeOwnerResult;
using IntQueue = simpledsp::util::QueueProducerConsumer<int>;
using TO = simpledsp::util::TimeOutMicrosSliced;

class TestObject {
  int uniqueValue_;
  IntQueue &destructed_;

public:
  TestObject(IntQueue &constructed, IntQueue &destructed)
      : uniqueValue_(rand()), destructed_(destructed) {
    constructed.put(uniqueValue_);
  }

  ~TestObject() { destructed_.put(uniqueValue_); }
};

BOOST_AUTO_TEST_SUITE(LockFreeOwnerTest)

BOOST_AUTO_TEST_CASE(testNotSetReturnsNull) {
  IntOwner owner;

  BOOST_CHECK(owner.get() == nullptr);
}

BOOST_AUTO_TEST_CASE(testGetReturnsSetSameThread) {
  IntOwner  owner;
  int *value = new int;
  *value = 15;
  TO t(1000000, 100);
  BOOST_CHECK_MESSAGE(owner.set(t, value) == Result::SUCCESS,
                      "Should be able to construct value");
  BOOST_CHECK(owner.get() == value);
}

BOOST_AUTO_TEST_CASE(testGetTheGetCurrentReturnsSetSameThread) {
  IntOwner owner;
  int *value = new int;
  *value = 15;
  TO t(1000000, 100);
  BOOST_CHECK_MESSAGE(owner.set(t, value) == Result::SUCCESS,
                      "Should be able to construct value");
  BOOST_CHECK(owner.get() == value);
  BOOST_CHECK(owner.getCurrent() == value);
}

BOOST_AUTO_TEST_CASE(testSetWithoutGetFailsSameThread) {
  IntOwner owner;
  int *value = new int;
  *value = 15;
  TO t(1000000, 100);
  BOOST_CHECK_MESSAGE(owner.set(t, value) == Result::SUCCESS,
                      "Should be able to construct value");
  int *value2 = new int;
  BOOST_CHECK(owner.set(t, value2) == Result::TIMEOUT);
  BOOST_CHECK(owner.get() == value);
  int *value3 = new int;
  BOOST_CHECK(owner.set(t, value3) == Result::SUCCESS);
}

BOOST_AUTO_TEST_CASE(testAllDestroyed) {
  IntQueue c(10);
  IntQueue d(10);
  TestObject *obj;
  {
    simpledsp::util::LockfreeOwner<TestObject> owner;
    TO t(1000000, 100);
    obj = new TestObject(c, d);
    owner.set(t, obj);
    BOOST_CHECK(owner.get() == obj);
    obj = new TestObject(c, d);
    owner.set(t, obj);
    BOOST_CHECK(owner.get() == obj);
    obj = new TestObject(c, d);
    owner.set(t, obj);
    BOOST_CHECK(owner.get() == obj);
  }
  BOOST_CHECK(c.size() == d.size());

  int value;
  std::string destructed = "|";
  while (d.get(value) == simpledsp::util::QueueResult::SUCCESS) {
    destructed += std::to_string(value);
    destructed += "|";
  }
  while (c.get(value) == simpledsp::util::QueueResult::SUCCESS) {
    std::string vstr = std::to_string(value);
    if (destructed.find(vstr) <= 0) {
      vstr += " constructed but not destructed: " + destructed;
      BOOST_CHECK_MESSAGE(false, vstr);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
