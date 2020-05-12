//
// Created by michel on 12-05-20.
//
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

/**
 * Never set these variables in code! These are meant to test the
 * test case:
 *  checkAlignment
 */
#define SIMPLE_CORE_SIMD_ALIGNMENT_COUNT 4
#define SIMPLE_CORE_SIMD_ALIGNMENT_TYPE 2

#include <simple-dsp/util/frame.h>

namespace {

static constexpr size_t FRAME_SIZE = 2;
typedef double vtype;
typedef simpledsp::Frame<vtype, FRAME_SIZE> Frame;

void set(Frame &frame, vtype v1, vtype v2) {
  frame[0] = v1;
  frame[1] = v2;
}

bool same(Frame &frame1, Frame &frame2) {
  for (size_t i = 0; i < Frame::size; i++) {
    if (frame1[i] != frame2[i]) {
      return false;
    }
  }
  return true;
}

} // namespace

BOOST_AUTO_TEST_SUITE(FrameTests)

/*
 * TODO: Much more test coverage!
 */


BOOST_AUTO_TEST_CASE(checkAlignment) {
  BOOST_CHECK_EQUAL(
      alignof(Frame), SIMPLE_CORE_SIMD_ALIGNMENT_COUNT * sizeof(vtype));
}

BOOST_AUTO_TEST_CASE(setAll) {
  Frame x;
  set(x, 13, 17);
  vtype value = 4;
  Frame expected;
  set(expected, value, value);
  x.set(value);
  BOOST_CHECK(same(x, expected));
}

BOOST_AUTO_TEST_CASE(testAddToAll) {
  Frame x;
  set(x, 13, 17);
  vtype add = 4;
  Frame expected;
  set(expected, x[0] + add, x[1] + add);
  x.add(add);
  BOOST_CHECK(same(x, expected));
}

BOOST_AUTO_TEST_SUITE_END()
