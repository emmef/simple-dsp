//
// Created by michel on 21-11-19.
//

#include <simple-dsp/guards.h>
#include <boost/test/unit_test.hpp>

static simpledsp::FlagGuard getGuard(std::atomic_flag &flag) {
  return simpledsp::FlagGuard(flag);
}


BOOST_AUTO_TEST_SUITE(SpinGuardTests)

  BOOST_AUTO_TEST_CASE(testFirstAndOnlyIsSet) {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    {
      simpledsp::FlagGuard guard(flag);

      BOOST_CHECK(guard.isSet());
      BOOST_CHECK(flag.test_and_set());
    }
    BOOST_CHECK_MESSAGE(!flag.test_and_set(), "Scope end dit not clear flag");
  }

  BOOST_AUTO_TEST_CASE(testFirstAndOnlyOutOfScopeIsNotSet) {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    {
      simpledsp::FlagGuard guard(flag);
      BOOST_CHECK(guard.isSet());
    }
    BOOST_CHECK(!flag.test_and_set());
  }

  BOOST_AUTO_TEST_CASE(testSecondNotSet) {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    {
      simpledsp::FlagGuard first(flag);
      simpledsp::FlagGuard second(flag);

      BOOST_CHECK(!second.isSet());
      BOOST_CHECK(flag.test_and_set());
    }
    BOOST_CHECK_MESSAGE(!flag.test_and_set(), "Scope end dit not clear flag");
  }

  BOOST_AUTO_TEST_CASE(testSecondNotSetThenClearedAndSetAgainIsSet) {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    simpledsp::FlagGuard first(flag);
    simpledsp::FlagGuard second(flag); // flag still set by first guard

    flag.clear();

    BOOST_CHECK(second.set());
  }

  BOOST_AUTO_TEST_CASE(testMoveTargetSetSourceNotSet) {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    {
      simpledsp::FlagGuard first(flag);
      simpledsp::FlagGuard second = std::move(first);

      BOOST_CHECK(second.isSet());
      BOOST_CHECK(!first.isSet());
      BOOST_CHECK(flag.test_and_set());
    }
    BOOST_CHECK_MESSAGE(!flag.test_and_set(), "Scope end dit not clear flag");
  }


  BOOST_AUTO_TEST_CASE(testFirstGuardFromFunctionIsSet) {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    {
      simpledsp::FlagGuard first = getGuard(flag);

      BOOST_CHECK(first.isSet());
      BOOST_CHECK(flag.test_and_set());
    }
    BOOST_CHECK_MESSAGE(!flag.test_and_set(), "Scope end dit not clear flag");
  }

BOOST_AUTO_TEST_SUITE_END()