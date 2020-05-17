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
typedef simpledsp::Frame<vtype, FRAME_SIZE, true> Frame;
typedef simpledsp::Frame<vtype, FRAME_SIZE, false> UFrame;

template<bool algn>
void set(simpledsp::Frame<vtype, FRAME_SIZE, algn> &frame, vtype v1, vtype v2) {
  frame[0] = v1;
  frame[1] = v2;
}

template<bool algn1, bool algn2>
bool same(simpledsp::Frame<vtype, FRAME_SIZE, algn1> &frame1, simpledsp::Frame<vtype, FRAME_SIZE, algn2> &frame2) {
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

BOOST_AUTO_TEST_CASE(testAddFrameToFrame) {
  Frame x;
  Frame y;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x + y;
  set(expected, 32, 46);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testAddUFrameToFrame) {
  Frame x;
  UFrame y;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x + y;
  set(expected, 32, 46);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testAddUFrameToFrameResultU) {
  Frame x;
  UFrame y;
  UFrame z;
  UFrame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x + y;
  set(expected, 32, 46);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testAddFrameToUFrame) {
  UFrame x;
  Frame y;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x + y;
  set(expected, 32, 46);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testAddFrameToUFrameResultU) {
  UFrame x;
  Frame y;
  UFrame z;
  UFrame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x + y;
  set(expected, 32, 46);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testSubFrameToFrame) {
  Frame x;
  Frame y;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x - y;
  set(expected, -6, -12);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testSubUFrameToFrame) {
  Frame x;
  UFrame y;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x - y;
  set(expected, -6, -12);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testSubUFrameToFrameResultU) {
  Frame x;
  UFrame y;
  UFrame z;
  UFrame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x - y;
  set(expected, -6, -12);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testSubFrameToUFrame) {
  UFrame x;
  Frame y;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x - y;
  set(expected, -6, -12);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testSubFrameToUFrameResultU) {
  UFrame x;
  Frame y;
  UFrame z;
  UFrame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x - y;
  set(expected, -6, -12);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testMulFrameWithValue) {
  Frame x;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  z = x * 4;
  set(expected, 52, 68);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testMulFrameWithValueAndAdd) {
  Frame x;
  Frame y;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = x * 4 + y;
  set(expected, 71, 97);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testAddToFrameWithAndMul) {
  Frame x;
  Frame y;
  Frame z;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  z = 4 * x + y;
  set(expected, 71, 97);
  BOOST_CHECK(same(z, expected));
}

BOOST_AUTO_TEST_CASE(testAssignSame) {
  Frame x;
  Frame y;
  Frame expected;
  printf("\n");
  set(x, 13, 17);
  set(y, 19, 29);
  y = x;
  set(expected, 13, 17);
  BOOST_CHECK(same(y, expected));
}

BOOST_AUTO_TEST_CASE(testAssignDifferent) {
  UFrame x;
  Frame y;
  Frame expected;
  set(x, 13, 17);
  set(y, 19, 29);
  y = x;
  set(expected, 13, 17);
  BOOST_CHECK(same(y, expected));
}



BOOST_AUTO_TEST_SUITE_END()
