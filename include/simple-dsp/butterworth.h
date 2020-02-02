#ifndef SIMPLE_DSP_BUTTERWORTH_H
#define SIMPLE_DSP_BUTTERWORTH_H
/*
 * simple-dsp/butterworth.h
 *
 * Added by michel on 2019-10-15
 * Copyright (C) 2015-2019 Michel Fleur.
 * Source https://github.com/emmef/simple-dsp
 * Email simple-dsp@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstring>
#include <simple-dsp/iir.h>
#include <simple-dsp/samplerate.h>
namespace simpledsp {

struct Butterworth {
  enum class Type { LOW_PASS, HIGH_PASS };

  static constexpr float FREQUENCY_RELATIVE_MINIMUM =
      std::numeric_limits<float>::epsilon();
  static constexpr float FREQUENCY__RELATIVE_MAXIMUM =
      0.5 - std::numeric_limits<float>::epsilon();
  static constexpr unsigned ORDER_MINIMUM = 1;
  static constexpr unsigned ORDER_MAXIMUM = 8;

  sdsp_nodiscard static const char *getTypeName(Type type) {
    return typeName(type);
  }

  sdsp_nodiscard static Butterworth
  relative(Type type,
           IIRCalculationMethod method = IIR_CALCULATION_METHOD_DEFAULT) {
    return Butterworth(type, 1.0, method);
  }

  sdsp_nodiscard static Butterworth
  forSampleRate(Type type, float sampleRate,
                IIRCalculationMethod method = IIR_CALCULATION_METHOD_DEFAULT) {
    return Butterworth(type, sampleRate, method);
  }

  explicit Butterworth(const Butterworth &source) = default;

  explicit Butterworth(Butterworth &&source) = default;

  sdsp_nodiscard const SampleRate &getSampleRate() const { return rate; }

  sdsp_nodiscard Type getType() const { return type; }

  sdsp_nodiscard float getFrequency() const { return frequency; }

  Butterworth &setCenter(float newCenter) {
    frequency = std::clamp(newCenter, FREQUENCY_RELATIVE_MINIMUM * rate,
                           FREQUENCY__RELATIVE_MAXIMUM * rate);
    return *this;
  }

  sdsp_nodiscard unsigned getOrder() const { return order; }

  Butterworth &setOrder(unsigned newOrder) {
    order = std::clamp(newOrder, ORDER_MINIMUM, ORDER_MAXIMUM);
    return *this;
  }

  void generate(IIRCoefficientsSetter &setter) const {
    switch (type) {
    case Type::LOW_PASS:
      getLowPassCoefficients(setter);
      return;
    case Type::HIGH_PASS:
      getHighPassCoefficients(setter);
      return;
    default:
      throw std::invalid_argument("Unknown filter pass (must be low or high");
    }
  }

  void generateOpposite(IIRCoefficientsSetter &setter) {
    switch (type) {
    case Type::HIGH_PASS:
      getLowPassCoefficients(setter);
      return;
    case Type::LOW_PASS:
      getHighPassCoefficients(setter);
      return;
    default:
      throw std::invalid_argument("Unknown filter pass (must be low or high");
    }
  }

private:
  static const char *typeName(Type type) {
    switch (type) {
    case Type::LOW_PASS:
      return "LOW_PASS";
    case Type::HIGH_PASS:
      return "HIGH_PASS";
    default:
      throw std::invalid_argument("Butterworth: invalid type");
    }
  }

  static Type verifyType(Type type) {
    typeName(type);
    return type;
  }

  explicit Butterworth(Type tp, float sampleRate, IIRCalculationMethod m)
      : type(verifyType(tp)), rate(sampleRate), frequency(rate / 4.0f),
        order(1), method(m) {}

  Type type;
  SampleRate rate;
  float frequency;
  unsigned order;
  IIRCalculationMethod method;

  void getLowPassCoefficients(IIRCoefficientsSetter &setter) const {
    float relativeFrequency = std::min(rate.relative(frequency), 0.5);
    if (order != setter.getOrder()) {
      setter.setOrder(order);
    }
    int unscaledCCoefficients[ORDER_MAXIMUM + 1];
    fill_with_zero(unscaledCCoefficients, sizeof(unscaledCCoefficients));
    getDCoefficients(order, relativeFrequency, method, setter);
    getUnscaledLowPassCCoefficients(order, unscaledCCoefficients);
    double scaleOfC = getLowPassScalingFactor(order, relativeFrequency);
    for (unsigned i = 0; i <= order; i++) {
      setter.setC(i, scaleOfC * unscaledCCoefficients[i]);
    }
  }

  void getHighPassCoefficients(IIRCoefficientsSetter &setter) const {
    float relativeFrequency = std::min(rate.relative(frequency), 0.5);
    if (order != setter.getOrder()) {
      setter.setOrder(order);
    }
    int unscaledCCoefficients[ORDER_MAXIMUM + 1];
    fill_with_zero(unscaledCCoefficients, sizeof(unscaledCCoefficients));
    getDCoefficients(order, relativeFrequency, method, setter);
    getUnscaledHighPassCCoefficients(order, unscaledCCoefficients);
    double scaleOfC = getHighPassScalingFactor(order, relativeFrequency);
    for (unsigned i = 0; i <= order; i++) {
      setter.setC(i, scaleOfC * unscaledCCoefficients[i]);
    }
  }

  template <typename T>
  static void fill_with_zero(T *const location, size_t size) {
    for (size_t i = 0; i < size / sizeof(T); i++) {
      location[i] = 0;
    }
  }

  static void getDCoefficients(unsigned order, double relativeFrequency,
                               IIRCalculationMethod method,
                               IIRCoefficientsSetter &d_coefficients) {
    double theta; // M_PI * relativeFrequency / 2.0
    double st;    // sine of theta
    double ct;    // cosine of theta
    double parg;  // pole angle
    double sparg; // sine of the pole angle
    double cparg; // cosine of the pole angle
    double a;     // workspace variable

    double dcof[ORDER_MAXIMUM * 2 + 1];
    fill_with_zero(dcof, sizeof(dcof));
    // binomial coefficients
    double binomials[2 * ORDER_MAXIMUM + 2];
    fill_with_zero(binomials, sizeof(binomials));

    theta = M_PI * relativeFrequency * 2;
    st = sin(theta);
    ct = cos(theta);

    for (unsigned k = 0; k < order; ++k) {
      parg = M_PI * (double)(2 * k + 1) / (double)(2 * order);
      sparg = sin(parg);
      cparg = cos(parg);
      a = 1.0 + st * sparg;
      binomials[2 * k] = -ct / a;
      binomials[2 * k + 1] = -st * cparg / a;
    }

    for (unsigned i = 0; i < order; ++i) {
      for (int j = i; j > 0; --j) {
        dcof[2 * j] += binomials[2 * i] * dcof[2 * (j - 1)] -
                       binomials[2 * i + 1] * dcof[2 * (j - 1) + 1];
        dcof[2 * j + 1] += binomials[2 * i] * dcof[2 * (j - 1) + 1] +
                           binomials[2 * i + 1] * dcof[2 * (j - 1)];
      }
      dcof[0] += binomials[2 * i];
      dcof[1] += binomials[2 * i + 1];
    }

    dcof[1] = dcof[0];
    dcof[0] = 1.0;
    for (unsigned k = 3; k <= order; ++k) {
      dcof[k] = dcof[2 * k - 2];
    }
    /*
     * Calculus results in coefficients that use subtraction of Y coefficients.
     */
    double sign = method == IIRCalculationMethod::POSITIVE_Y ? -1.0 : 1.0;
    for (unsigned i = 0; i <= order; i++) {
      d_coefficients.setD(i, sign * dcof[i]);
    }
  }

  static void getUnscaledLowPassCCoefficients(int order, int *ccof) {
    ccof[0] = 1;
    ccof[1] = order;

    for (int m = order / 2, i = 2; i <= m; ++i) {
      ccof[i] = (order - i + 1) * ccof[i - 1] / i;
      ccof[order - i] = ccof[i];
    }
    ccof[order - 1] = order;
    ccof[order] = 1;
  }

  static double getLowPassScalingFactor(unsigned order,
                                        double relativeFrequency) {
    unsigned k;    // loop variables
    double omega;  // M_PI * relativeFrequency
    double fomega; // function of omega
    double parg0;  // zeroth pole angle
    double sf;     // scaling factor

    omega = M_PI * relativeFrequency * 2;
    fomega = sin(omega);
    parg0 = M_PI / (double)(2 * order);

    sf = 1.0;
    for (k = 0; k < order / 2; ++k) {
      sf *= 1.0 + fomega * sin((double)(2 * k + 1) * parg0);
    }

    fomega = sin(omega / 2.0);

    if (order % 2) {
      sf *= fomega + cos(omega / 2.0);
    }
    sf = pow(fomega, order) / sf;

    return (sf);
  }

  static double getHighPassScalingFactor(unsigned order,
                                         double relativeFrequency) {
    unsigned k;    // loop variables
    double omega;  // M_PI * relativeFrequency
    double fomega; // function of omega
    double parg0;  // zeroth pole angle
    double sf;     // scaling factor

    omega = M_PI * relativeFrequency * 2;
    fomega = sin(omega);
    parg0 = M_PI / (double)(2 * order);

    sf = 1.0;
    for (k = 0; k < order / 2; ++k) {
      sf *= 1.0 + fomega * sin((double)(2 * k + 1) * parg0);
    }

    fomega = cos(omega / 2.0);

    if (order % 2) {
      sf *= fomega + sin(omega / 2.0);
    }
    sf = pow(fomega, order) / sf;

    return (sf);
  }

  static void getUnscaledHighPassCCoefficients(size_t order, int *ccof) {
    getUnscaledLowPassCCoefficients(order, ccof);

    for (size_t i = 0; i <= order; ++i) {
      if (i % 2) {
        ccof[i] = -ccof[i];
      }
    }
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_BUTTERWORTH_H
