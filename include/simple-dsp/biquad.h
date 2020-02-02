#ifndef SIMPLE_DSP_BIQUAD_H
#define SIMPLE_DSP_BIQUAD_H
/*
 * simple-dsp/biquad.h
 *
 * Added by michel on 2019-10-14
 * Copyright (C) 2015-2019 Michel Fleur.
 * Source https://github.com/emmef/simple-dsp
 * Email simple-dsp@emmef.org
 *
 * Parts of this code where adapted from the swh LADSPA plugin project:
 * https://github.com/swh/ladspa/blob/master/AUTHORS.
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

struct Biquad {
  enum class Type {
    PARAMETRIC,
    SHELVE_LOW,
    SHELVE_HIGH,
    PASS_LOW,
    PASS_HIGH,
    PASS_BAND
  };

  static constexpr float BANDWITH_MINIMUM = 0.0625;
  static constexpr float BANDWITH_MAXIMUM = 16;
  static constexpr float SLOPE_MINIMUM = 0.0001;
  static constexpr float SLOPE_MAXIMUM = 1;
  static constexpr float CENTER_RELATIVE_MINIMUM =
      std::numeric_limits<float>::epsilon();
  static constexpr float CENTER_RELATIVE_MAXIMUM =
      0.5 - std::numeric_limits<float>::epsilon();
  static constexpr float GAIN_MINIMUM = 0.01;
  static constexpr float GAIN_MAXIMUM = 100;
  static constexpr const char *TYPES_WITH_GAIN =
      "|PARAMETRIC|SHELVE_HIGH|SHELVE_LOW|";
  static constexpr const char *TYPES_WITH_SLOPE = "|SHELVE_HIGH|SHELVE_LOW|";
  static constexpr const char *TYPES_WITH_BANDWIDTH =
      "|PARAMETRIC|PASS_BAND|PASS_LOW|PASS_HIGH|";

  sdsp_nodiscard static const char *getTypeName(Type type) {
    return typeName(type);
  }

  sdsp_nodiscard static Biquad
  relative(Type type,
           IIRCalculationMethod method = IIR_CALCULATION_METHOD_DEFAULT) {
    return Biquad(type, 1.0, method);
  }

  sdsp_nodiscard static Biquad
  forSampleRate(Type type, float sampleRate,
                IIRCalculationMethod method = IIR_CALCULATION_METHOD_DEFAULT) {
    return Biquad(type, sampleRate, method);
  }

  explicit Biquad(const Biquad &source) = default;

  explicit Biquad(Biquad &&source) = default;

  sdsp_nodiscard const SampleRate &getSampleRate() const { return rate; }

  sdsp_nodiscard Type getType() const { return type; }

  sdsp_nodiscard float getCenter() const { return center; }

  Biquad &setCenter(float newCenter) {
    center = std::clamp(newCenter, CENTER_RELATIVE_MINIMUM * rate,
                        CENTER_RELATIVE_MAXIMUM * rate);
    return *this;
  }

  sdsp_nodiscard float getGain() const {
    checkAccess("gain factor", type, TYPES_WITH_GAIN);
    return gain;
  }

  Biquad &setGain(float newGain) {
    checkAccess("gain factor", type, TYPES_WITH_GAIN);
    gain = std::clamp(newGain, GAIN_MINIMUM, GAIN_MAXIMUM);
    return *this;
  }

  sdsp_nodiscard float getSlope() const {
    checkAccess("slope", type, TYPES_WITH_SLOPE);
    return widthOrSlope;
  }

  Biquad &setSlope(float newSlope) {
    checkAccess("slope", type, TYPES_WITH_SLOPE);
    widthOrSlope = std::clamp(newSlope, SLOPE_MINIMUM, SLOPE_MAXIMUM);
    return *this;
  }

  sdsp_nodiscard float getBandwidth() const {
    checkAccess("bandwidth", type, TYPES_WITH_BANDWIDTH);
    return widthOrSlope;
  }

  Biquad &setBandwidth(float newBandwidth) {
    checkAccess("bandwidth", type, TYPES_WITH_BANDWIDTH);
    widthOrSlope = std::clamp(newBandwidth, BANDWITH_MINIMUM, BANDWITH_MAXIMUM);
    return *this;
  }

  void generate(IIRCoefficientsSetter &coefficients) {
    if (coefficients.getOrder() != 2) {
      coefficients.setOrder(2);
    }
    switch (type) {
    case Type::PARAMETRIC:
      generateParametric(coefficients);
    case Type::SHELVE_HIGH:
      generateHighShelve(coefficients);
    case Type::SHELVE_LOW:
      generateLowShelve(coefficients);
    case Type::PASS_BAND:
      generateBandPass(coefficients);
    case Type::PASS_HIGH:
      generateHighPass(coefficients);
    case Type::PASS_LOW:
      generateLowPass(coefficients);
    default:
      throw std::invalid_argument("Biquad: invalid type");
    }
  }

private:
  static const char *typeName(Type type) {
    switch (type) {
    case Type::PARAMETRIC:
      return "PARAMETRIC";
    case Type::SHELVE_HIGH:
      return "SHELVE_HIGH";
    case Type::SHELVE_LOW:
      return "SHELVE_LOW";
    case Type::PASS_BAND:
      return "PASS_BAND";
    case Type::PASS_HIGH:
      return "PASS_HIGH";
    case Type::PASS_LOW:
      return "PASS_LOW";
    default:
      throw std::invalid_argument("Biquad: invalid type");
    }
  }

  static Type verifyType(Type type) {
    typeName(type);
    return type;
  }

  static void checkAccess(const char *whatToAccess, Type type,
                          const char *allowedTypes) {
    const char *name = typeName(type);
    if (strstr(allowedTypes, name) == nullptr) {
      std::string message = "Biquad(";
      message += name;
      message += ") has no ";
      message += whatToAccess;
      throw std::runtime_error(message);
    }
  }

  explicit Biquad(Type tp, float sampleRate, IIRCalculationMethod m)
      : type(verifyType(tp)), rate(sampleRate), center(rate / 4.0f), gain(1.0),
        widthOrSlope(1.0), method(m) {}

  struct BiQuadCoefficients {
    double C0;
    double C1;
    double C2;
    double D1;
    double D2;
  };

  void setCoefficients(IIRCoefficientsSetter &setter,
                       const BiQuadCoefficients bqc) {
    setter.setC(0, bqc.C0);
    setter.setC(1, bqc.C1);
    setter.setC(2, bqc.C2);
    setter.setD(0, 0);
    setter.setD(1,
                method == IIRCalculationMethod::POSITIVE_Y ? bqc.D1 : -bqc.D1);
    setter.setD(2,
                method == IIRCalculationMethod::POSITIVE_Y ? bqc.D2 : -bqc.D2);
  }

  void generateParametric(IIRCoefficientsSetter &setter) {
    static constexpr double LN_2_2 = (M_LN2 / 2);
    double omega = rate.relativeAngular(center);
    double cw = cosf(omega);
    double sw = sinf(omega);
    double J = sqrt(gain);
    double g = sw * sinhf(LN_2_2 * widthOrSlope * omega / sw);
    double a0r = 1.0f / (1.0f + (g / J));

    BiQuadCoefficients result;
    result.C0 = (1.0f + (g * J)) * a0r;
    result.C1 = (-2.0f * cw) * a0r;
    result.C2 = (1.0f - (g * J)) * a0r;
    result.D1 = -result.C1;
    result.D2 = ((g / J) - 1.0f) * a0r;

    setCoefficients(setter, result);
  }

  void generateHighShelve(IIRCoefficientsSetter &setter) {
    double omega = rate.relativeAngular(center);
    double cw = cosf(omega);
    double sw = sinf(omega);
    double A = sqrt(gain);
    double b =
        sqrt(((1.0f + A * A) / widthOrSlope) - ((A - 1.0f) * (A - 1.0f)));
    double apc = cw * (A + 1.0f);
    double amc = cw * (A - 1.0f);
    double bs = b * sw;
    double a0r = 1.0f / (A + 1.0f - amc + bs);

    BiQuadCoefficients result;
    result.C0 = a0r * A * (A + 1.0f + amc + bs);
    result.C1 = a0r * -2.0f * A * (A - 1.0f + apc);
    result.C2 = a0r * A * (A + 1.0f + amc - bs);
    result.D1 = a0r * -2.0f * (A - 1.0f - apc);
    result.D2 = a0r * (-A - 1.0f + amc + bs);

    setCoefficients(setter, result);
  }

  void generateLowShelve(IIRCoefficientsSetter &setter) {
    double omega = rate.relativeAngular(center);
    double cw = cosf(omega);
    double sw = sinf(omega);
    double A = sqrt(gain);
    double b = sqrt(((1.0f + A * A) / widthOrSlope) - ((A - 1.0) * (A - 1.0)));
    double apc = cw * (A + 1.0f);
    double amc = cw * (A - 1.0f);
    double bs = b * sw;
    double a0r = 1.0f / (A + 1.0f + amc + bs);

    BiQuadCoefficients result;
    result.C0 = a0r * A * (A + 1.0f - amc + bs);
    result.C1 = a0r * 2.0f * A * (A - 1.0f - apc);
    result.C2 = a0r * A * (A + 1.0f - amc - bs);
    result.D1 = a0r * 2.0f * (A - 1.0f + apc);
    result.D2 = a0r * (-A - 1.0f - amc + bs);

    setCoefficients(setter, result);
  }

  void generateBandPass(IIRCoefficientsSetter &setter) {
    double omega = rate.relativeAngular(center);
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha = sn * sinh(M_LN2 / 2.0 * widthOrSlope * omega / sn);
    const double a0r = 1.0 / (1.0 + alpha);

    BiQuadCoefficients result;
    result.C0 = a0r * alpha;
    result.C1 = 0.0;
    result.C2 = a0r * -alpha;
    result.D1 = a0r * (2.0 * cs);
    result.D2 = a0r * (alpha - 1.0);

    setCoefficients(setter, result);
  }

  void generateHighPass(IIRCoefficientsSetter &setter) {
    double omega = rate.relativeAngular(center);
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha = sn * sinh(M_LN2 / 2.0 * widthOrSlope * omega / sn);
    const double a0r = 1.0 / (1.0 + alpha);

    BiQuadCoefficients result;
    result.C0 = a0r * (1.0 + cs) * 0.5;
    result.C1 = a0r * -(1.0 + cs);
    result.C2 = a0r * (1.0 + cs) * 0.5;
    result.D1 = a0r * (2.0 * cs);
    result.D2 = a0r * (alpha - 1.0);

    setCoefficients(setter, result);
  }

  void generateLowPass(IIRCoefficientsSetter &setter) {
    double omega = rate.relativeAngular(center);
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha = sn * sinh(M_LN2 / 2.0 * widthOrSlope * omega / sn);
    const double a0r = 1.0 / (1.0 + alpha);

    BiQuadCoefficients result;
    result.C0 = a0r * (1.0 - cs) * 0.5;
    result.C1 = a0r * (1.0 - cs);
    result.C2 = a0r * (1.0 - cs) * 0.5;
    result.D1 = a0r * (2.0 * cs);
    result.D2 = a0r * (alpha - 1.0);

    setCoefficients(setter, result);
  }

  Type type;
  SampleRate rate;
  float center;
  float gain;
  float widthOrSlope;
  IIRCalculationMethod method;
};
} // namespace simpledsp

#endif // SIMPLE_DSP_BIQUAD_H
