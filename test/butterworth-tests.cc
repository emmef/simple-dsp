//
// Created by michel on 16-10-19.
//

#include <vector>
#include <simple-dsp/butterworth.h>
#include "test-helper.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace {

  using namespace std;
  using Coefficients = simpledsp::IIRCoefficients<float>;
  using Filter = simpledsp::Butterworth;


  struct Scenario {
    static constexpr float epsilon = 1e-4;

    static bool same(float expected, float actual) {
      if (expected == 0) {
        return fabs(actual) <= epsilon;
      }
      return fabs(expected - actual) / fabs(expected) <= epsilon;
    }

    Filter designer;
    Coefficients expectedCoefficients;

    Scenario(const Filter &filter, const Coefficients &expected) :
            designer(filter),
            expectedCoefficients(expected) {

    }

    void test() const;
    bool verifyEqual(const char coefficient[2], size_t index, float expected, float actual) const;
  };

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
namespace std {
  ostream& operator<<(ostream& stream, const Filter& s) {
    stream
            << "Butterworth{order=" << s.getOrder()
            << "; frequency=" << s.getFrequency()
            << "; sample rate=" << s.getSampleRate()
            << "; type=" << Filter::getTypeName(s.getType())
            << "}";
    return stream;
  }

  ostream& operator<<(ostream& stream, const Scenario& s) {
    stream
            << "Scenario={" << s.designer << "}";
    return stream;
  }
}
#pragma clang diagnostic pop


void Scenario::test() const {
  Coefficients actualCoefficients(expectedCoefficients.getOrder());

  simpledsp::IIRCoefficients<float>::Setter setter = actualCoefficients.setter();
  designer.generate(setter);
  BOOST_CHECK_MESSAGE(
          actualCoefficients.getOrder() == expectedCoefficients.getOrder(),
          "Order of coefficient not as expected");
  bool same = true;
  for (size_t i = 0; i <= expectedCoefficients.getOrder(); i++) {
    same &= Scenario::same(expectedCoefficients.getC(i), actualCoefficients.getC(i));
    same &= Scenario::same(expectedCoefficients.getD(i), actualCoefficients.getD(i));
  }
  if (!same) {
    BOOST_CHECK(false);
    for (size_t i = 0; i <= expectedCoefficients.getOrder(); i++) {
      if (!Scenario::same(expectedCoefficients.getC(i), actualCoefficients.getC(i))) {
        cout << "\tC[" << i << "] expected " << expectedCoefficients.getC(i)
             << " actual " << actualCoefficients.getC(i) << endl;
      }
      if (!Scenario::same(expectedCoefficients.getD(i), actualCoefficients.getD(i))) {
        cout << "\tD[" << i << "] expected " << expectedCoefficients.getD(i)
             << " actual " << actualCoefficients.getD(i) << endl;
      }
    }
  }
}
bool Scenario::verifyEqual(const char* coefficient,
        size_t index,
        float expected,
        float actual) const {
  bool equal = Scenario::same(expected, actual);
  if (!equal) {
    BOOST_CHECK_EQUAL(expected, actual);
    cout << "\tCoefficient " << coefficient << "[" << index << "]" << endl;
    return false;
  }
  return true;
}

vector<Scenario> &scenarios() {
  static vector<Scenario> result;
  Coefficients coefficients(8u);
  auto setter = coefficients.setter();

  setter.setOrder(1)
          .setC(0, 0.57919).setC(1, 0.57919)
          .setD(0, -1.00000).setD(1, -0.15838);
  result.push_back({
          Filter::relative(
                  Filter::Type::LOW_PASS).setOrder(1).setCenter(0.3),
          coefficients});

  setter.setOrder(1)
          .setC(0, 0.42081).setC(1, -0.42081)
          .setD(0, -1.00000).setD(1, -0.15838);
  result.push_back({
          Filter::relative(
                  Filter::Type::HIGH_PASS).setOrder(1).setCenter(0.3),
          coefficients});

  setter.setOrder(2)
          .setC(0, 0.39134).setC(1, 0.78267).setC(2, 0.39134)
          .setD(0, -1.00000).setD(1, -0.36953).setD(2, -0.19582);
  result.push_back({
          Filter::relative(
                  Filter::Type::LOW_PASS).setOrder(2).setCenter(0.3),
          coefficients});

  setter.setOrder(2)
          .setC(0, 0.20657).setC(1, -0.41314).setC(2, 0.20657)
          .setD(0, -1.00000).setD(1, -0.36953).setD(2, -0.19582);
  result.push_back({
          Filter::relative(
                  Filter::Type::HIGH_PASS).setOrder(2).setCenter(0.3),
          coefficients});

  return result;
}


BOOST_AUTO_TEST_SUITE(iirIntegrationTest)


  BOOST_DATA_TEST_CASE(sample, scenarios()) {
    sample.test();
  }

BOOST_AUTO_TEST_SUITE_END()
