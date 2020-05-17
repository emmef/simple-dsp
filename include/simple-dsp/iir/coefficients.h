#ifndef SIMPLE_DSP_IIR_COEFFICIENTS_H
#define SIMPLE_DSP_IIR_COEFFICIENTS_H
/*
 * simple-dsp/coefficients.h
 *
 * Added by michel on 2020-05-17
 * Copyright (C) 2015-2020 Michel Fleur.
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

#include <cstddef>
#include <simple-dsp/core/alignment.h>
#include <stdexcept>

namespace simpledsp::iir {

static constexpr size_t IIR_MAX_ORDER = 31;

static bool is_valid_order(size_t order) {
  return order > 0 && order <= IIR_MAX_ORDER;
}

static size_t get_valid_order(size_t order) {
  if (is_valid_order(order)) {
    return order;
  }
  throw std::invalid_argument("get_valid_order: invalid order");
}

/**
 * Describes the convention used to calculate filters, that is based on whether
 * coefficients related to output values (Y), are added or subtracted.
 */
enum class CoefficientConvention { POSITIVE_Y, NEGATIVE_Y };

/**
 * Allows algorithms that calculate filter coefficients to set them in a
 * coefficient implementation. The implementation can remain highly optimised
 * but setting the calculated coefficients can be done in a standard way.
 *
 * @tparam T type of coefficent values
 */
class CoefficientsSetter {

  double getConventionCorrectedY(double v,
                                 CoefficientConvention caller) {
    return caller == CoefficientConvention::POSITIVE_Y ? v : -v;
  }

protected:
  virtual void setValidOrder(unsigned order) = 0;
  virtual void setValidX(size_t i, double value) = 0;
  virtual void setValidY(size_t i, double value) = 0;

public:
  sdsp_nodiscard virtual unsigned getOrder() const = 0;
  sdsp_nodiscard virtual unsigned getMaxOrder() const = 0;
  sdsp_nodiscard virtual bool canSetOrder() const = 0;
  virtual ~CoefficientsSetter() = default;

  sdsp_nodiscard size_t getCoefficients() const { return getOrder() + 1; }

  CoefficientsSetter &setOrder(unsigned order) {
    if (order == 0) {
      throw std::invalid_argument("IIRCoefficientsSetter::setOrder(order): "
                                  "order must be 1 or higher.");
    }
    if (!canSetOrder()) {
      if (order == getOrder()) {
        return *this;
      }
      throw std::runtime_error(
          "IIRCoefficientsSetter::setOrder(order): "
          "implementation does not allow change of filter order.");
    }
    if (order > getMaxOrder()) {
      throw std::invalid_argument(
          "IIRCoefficientsSetter::setOrder(order): "
          "order cannot exceed maximum of implementation.");
    }
    setValidOrder(order);
    return *this;
  }

  bool setOrderGetSuccess(unsigned order) {
    if (order == 0) {
      return false;
    }
    if (!canSetOrder() && (order != getOrder())) {
      return false;
    }
    if (order > getMaxOrder()) {
      return false;
    }
    setValidOrder(order);
    return true;
  }

  CoefficientsSetter &setX(size_t i, double value) {
    setValidX(Index::checked(i, getCoefficients()), value);
    return *this;
  }

  CoefficientsSetter &setY(size_t i, double value,
                           CoefficientConvention convention) {
    setValidY(Index::checked(i, getCoefficients()),
              getConventionCorrectedY(value, convention));
    return *this;
  }

  CoefficientsSetter &amplifyOnly(double scale = 1.0) noexcept {
    setValidX(0, scale);
    setValidY(0, 0.0);
    for (size_t i = 0; i <= getOrder(); i++) {
      setValidX(i, 0.0);
      setValidY(i, 0.0);
    }
    return *this;
  }
};

/**
 * Wraps access to coefficient implementation so that algorithms do not need to
 * care about internals nor have to dealwith polymorphism.
 * @tparam coeff Type of coefficient, that MUST be a floating-point.
 * @tparam CoefficientsClass A class that contains coefficients.
 */
template <typename coeff, class CoefficientsClass> class CoefficientsGetter {
  static_assert(std::is_floating_point_v<coeff>,
                "IIRCoefficientAccess<coeff, CoefficientsClass>: "
                "coeff must be floating-point");

  sdsp_nodiscard sdsp_force_inline static const coeff &
  getX(size_t i, const CoefficientsClass &coeffs) {
    return coeffs.getX(i);
  }
  sdsp_nodiscard sdsp_force_inline static const coeff &
  getY(size_t i, const CoefficientsClass &coeffs) {
    return coeffs.getY(i);
  }
  sdsp_nodiscard sdsp_force_inline static constexpr unsigned
  getOrder(const CoefficientsClass &coeffs) {
    return coeffs.getOrder();
  }
};

template <typename sample, typename coeff>
sdsp_nodiscard sdsp_force_inline sample
filter_single(size_t order, sample *__restrict xHistory,
              sample *__restrict yHistory, const coeff *__restrict coeffX,
              const coeff *__restrict coeffY, const sample x) {

  sample Y = 0;
  sample X = x; // input is xN0
  sample yN0 = 0.0;
  size_t i, j;
  for (i = 0, j = 1; i < order; i++, j++) {
    const sample xN1 = xHistory[i];
    const sample yN1 = yHistory[i];
    xHistory[i] = X;
    X = xN1;
    yHistory[i] = Y;
    Y = yN1;
    yN0 += xN1 * coeffX[j] + yN1 * coeffY[j];
  }
  yN0 += coeffX[0] * x;

  yHistory[0] = yN0;

  return yN0;
}

template <typename sample, typename coeff, size_t N>
static void filter_aligned_forward(size_t order, const coeff *__restrict coeffX,
                                   const coeff *__restrict coeffY,
                                   const sample *__restrict x,
                                   const sample *__restrict y, size_t count) {
  const sample *__restrict x_;
  sample *__restrict y_;
  if constexpr (N > 1) {
    static_assert(Power2::is(N), "Alignment must be power of two");
    x_ = assume_aligned<N>(x);
    y_ = assume_aligned<N>(y);
  } else {
    x_ = x;
    y_ = y;
  }

  for (size_t n = 0; n < count; n++) {
    sample yN = coeffX[0] * x_[n];
    for (size_t j = 1; j <= order; j++) {
      yN += x_[n - j] * coeffX[j] + y_[n - j] * coeffY[j];
    }
    y_[n] = yN;
  }
}

template <typename sample, typename coeff, size_t N>
static void filter_aligned_backwards(size_t order,
                                     const coeff *__restrict coeffX,
                                     const coeff *__restrict coeffY,
                                     const sample *__restrict x,
                                     const sample *__restrict y, size_t count) {
  const sample *__restrict x_;
  sample *__restrict y_;
  if constexpr (N > 1) {
    static_assert(Power2::is(N), "Alignment must be power of two");
    x_ = assume_aligned<N>(x);
    y_ = assume_aligned<N>(y);
  } else {
    x_ = x;
    y_ = y;
  }

  for (ptrdiff_t n = count - 1; n >= 0; n--) {
    sample yN = coeffX[0] * x_[n];
    for (size_t j = 1; j <= order; j++) {
      yN += x_[n + j] * coeffX[j] + y_[n + j] * coeffY[j];
    }
    y_[n] = yN;
  }
}

template <typename sample, typename coeff, size_t order>
sdsp_nodiscard sdsp_force_inline sample
filter_single(sample *__restrict xHistory, sample *__restrict yHistory,
              const coeff *__restrict coeffX, const coeff *__restrict coeffY,
              const sample x) {

  sample Y = 0;
  sample X = x; // input is xN0
  sample yN0 = 0.0;
  size_t i, j;
  for (i = 0, j = 1; i < order; i++, j++) {
    const sample xN1 = xHistory[i];
    const sample yN1 = yHistory[i];
    xHistory[i] = X;
    X = xN1;
    yHistory[i] = Y;
    Y = yN1;
    yN0 += xN1 * coeffX[j] + yN1 * coeffY[j];
  }
  yN0 += coeffX[0] * x;

  yHistory[0] = yN0;

  return yN0;
}

template <typename sample, typename coeff, size_t N, size_t order>
static void filter_aligned_backwards(const coeff *__restrict coeffX,
                                     const coeff *__restrict coeffY,
                                     const sample *__restrict x,
                                     const sample *__restrict y, size_t count) {
  const sample *__restrict x_;
  sample *__restrict y_;
  if constexpr (N > 1) {
    static_assert(Power2::is(N), "Alignment must be power of two");
    x_ = assume_aligned<N>(x);
    y_ = assume_aligned<N>(y);
  } else {
    x_ = x;
    y_ = y;
  }

  for (ptrdiff_t n = count - 1; n >= 0; n--) {
    sample yN = coeffX[0] * x_[n];
    for (size_t j = 1; j <= order; j++) {
      yN += x_[n + j] * coeffX[j] + y_[n + j] * coeffY[j];
    }
    y_[n] = yN;
  }
}

template <typename sample, typename coeff, size_t N, size_t order>
static void filter_aligned_forward(const coeff *__restrict coeffX,
                                   const coeff *__restrict coeffY,
                                   const sample *__restrict x,
                                   const sample *__restrict y, size_t count) {
  const sample *__restrict x_;
  sample *__restrict y_;
  if constexpr (N > 1) {
    static_assert(Power2::is(N), "Alignment must be power of two");
    x_ = assume_aligned<N>(x);
    y_ = assume_aligned<N>(y);
  } else {
    x_ = x;
    y_ = y;
  }

  for (size_t n = 0; n < count; n++) {
    sample yN = coeffX[0] * x_[n];
    for (size_t j = 1; j <= order; j++) {
      yN += x_[n - j] * coeffX[j] + y_[n - j] * coeffY[j];
    }
    y_[n] = yN;
  }
}

template <typename C>
class VariableOrderCoefficients : public CoefficientsSetter {
  size_t maxOrder;
  size_t order;
  C *coeffX;
  C *coeffY;

protected:
  void setValidOrder(unsigned newOrder) override {
    order = newOrder;
    coeffY = coeffX + order + 1;
  }

  void setValidX(size_t i, double value) override { coeffX[i] = value; }

  void setValidY(size_t i, double value) override { coeffY[i] = value; }

public:
  VariableOrderCoefficients(size_t max_order)
      : maxOrder(get_valid_order(max_order)),
        coeffX(new C[2 * (max_order + 1)]), order(maxOrder) {}

  sdsp_nodiscard virtual unsigned getOrder() const override { return order; }
  sdsp_nodiscard virtual unsigned getMaxOrder() const override {
    return maxOrder;
  }
  sdsp_nodiscard virtual bool canSetOrder() const override { return true; }

  template <typename sample>
  void filter(const sample *__restrict x, const sample *__restrict y,
              size_t count) const {
    filterAligned<sample, alignof(sample)>(x, y, count);
  }

  template <typename sample>
  void filterVec(const sample *__restrict x, const sample *__restrict y,
                 size_t count) const {
    static constexpr AlignedFor<sample> alignment;
    filterAligned<sample, alignment.bytes>(x, y, count);
  }

  template <typename sample, size_t N>
  void filterAligned(const sample *__restrict x, const sample *__restrict y,
                     size_t count) const {
    filter_aligned_forward<N>(order, coeffX, coeffY, x, y, count);
  }

  template <typename sample>
  void filterBackwards(const sample *__restrict x, const sample *__restrict y,
                       size_t count) const {
    filterAlignedBackwards<sample, alignof(sample)>(x, y, count);
  }

  template <typename sample>
  void filterVecBackwards(const sample *__restrict x,
                          const sample *__restrict y, size_t count) const {
    static constexpr AlignedFor<sample> alignment;
    filterAlignedBackwards<sample, alignment.bytes>(x, y, count);
  }

  template <typename sample, size_t N>
  void filterAlignedBackwards(const sample *__restrict x,
                              const sample *__restrict y, size_t count) const {
    filter_aligned_backwards<N>(order, coeffX, coeffY, x, y, count);
  }

  template <typename sample>
  sdsp_nodiscard sdsp_force_inline sample filter(sample *__restrict xHistory,
                                                 sample *__restrict yHistory,
                                                 const sample x) {
    return filter_single(order, xHistory, yHistory, coeffX, coeffY, x);
  }
};

template <typename C, size_t O>
class FixedOrderCoefficients : public CoefficientsSetter {
  static constexpr size_t OFFSET = O + 1;
  C coeffX[2 * OFFSET];

protected:
  void setValidOrder(unsigned) override {}

  void setValidX(size_t i, double value) override { coeffX[i] = value; }

  void setValidY(size_t i, double value) override {
    coeffX[i + OFFSET] = value;
  }

public:
  FixedOrderCoefficients() {}

  sdsp_nodiscard virtual unsigned getOrder() const override { return O; }
  sdsp_nodiscard virtual unsigned getMaxOrder() const override { return O; }
  sdsp_nodiscard virtual bool canSetOrder() const override { return false; }

  template <typename sample>
  void filter(const sample *__restrict x, const sample *__restrict y,
              size_t count) const {
    filterAligned<sample, alignof(sample)>(x, y, count);
  }

  template <typename sample>
  void filterVec(const sample *__restrict x, const sample *__restrict y,
                 size_t count) const {
    static constexpr AlignedFor<sample> alignment;
    filterAligned<sample, alignment.bytes>(x, y, count);
  }

  template <typename sample, size_t N>
  void filterAligned(const sample *__restrict x, const sample *__restrict y,
                     size_t count) const {
    filter_aligned_forward<N>(O, coeffX, coeffX + OFFSET, x, y, count);
  }

  template <typename sample>
  void filterBackwards(const sample *__restrict x, const sample *__restrict y,
                       size_t count) const {
    filterAlignedBackwards<sample, alignof(sample)>(x, y, count);
  }

  template <typename sample>
  void filterVecBackwards(const sample *__restrict x,
                          const sample *__restrict y, size_t count) const {
    static constexpr AlignedFor<sample> alignment;
    filterAlignedBackwards<sample, alignment.bytes>(x, y, count);
  }

  template <typename sample, size_t N>
  void filterAlignedBackwards(const sample *__restrict x,
                              const sample *__restrict y, size_t count) const {
    filter_aligned_backwards<N>(O, coeffX, coeffX + OFFSET, x, y, count);
  }

  template <typename sample>
  sdsp_nodiscard sdsp_force_inline sample filter(sample *__restrict xHistory,
                                                 sample *__restrict yHistory,
                                                 const sample x) {
    return filter_single(xHistory, yHistory, coeffX, coeffX + OFFSET, x);
  }
};

} // namespace simpledsp::iir

#endif // SIMPLE_DSP_IIR_COEFFICIENTS_H
