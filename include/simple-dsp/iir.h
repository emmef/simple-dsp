#ifndef SIMPLE_DSP_IIR_H
#define SIMPLE_DSP_IIR_H
/*
 * simple-dsp/iir.h
 *
 * Added by michel on 2019-10-13
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
#include <simple-dsp/core/attributes.h>
#include <simple-dsp/core/index.h>
#include <simple-dsp/iir/coefficients.h>

namespace simpledsp {

/**
 * A tagging interface that contains no methods but is necessary to use a class
 * in an iir::CoefficientsGetter wrapper.
 */
struct IIRCoefficientsClass {};

typedef iir::CoefficientConvention IIRCalculationMethod;

template <typename coeff, class sample, IIRCalculationMethod method>
struct IIRSingleCalculation;

template <typename coeff, class sample>
struct IIRSingleCalculation<coeff, sample, IIRCalculationMethod::NEGATIVE_Y> {

  sdsp_force_inline static void iterate(coeff *out, const coeff &coeffX,
                                        const sample *x, const coeff &coeffY,
                                        const sample *y, size_t count) {
    for (size_t i = 0; i < count; i++) {
      out[i] = x[i] * coeffX - y[i] * coeffY;
    }
  }
};

template <typename coeff, class sample>
struct IIRSingleCalculation<coeff, sample, IIRCalculationMethod::POSITIVE_Y> {
  sdsp_force_inline static void iterate(coeff *out, const coeff &coeffX,
                                        const sample *x, const coeff &coeffY,
                                        const sample *y, size_t count) {
    for (size_t i = 0; i < count; i++) {
      out[i] = x[i] * coeffX + y[i] * coeffY;
    }
  }
};

/**
 * Using the coefficient parameter and method of calculation, delivers various
 * ways to calculate IIR filters.
 * @tparam coeff The coefficient value type
 * @tparam method The way to calculate the filter with given coefficients
 */
template <typename coeff>
struct IIRFilterBase {

  /**
   * Calculates the next output value for the input value @code{x}.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @param filter The filter coefficients
   * @param xHistory State: the previous input values (at least as much as the
   * order)
   * @param yHistory State: the previous output values (at least as much as the
   * order)
   * @param x the input value
   * @return the calculated output value
   */
  template <class Filter, typename sample>
  sdsp_nodiscard sdsp_force_inline static sample
  withHistory(const Filter &filter, sample *__restrict xHistory,
              sample *__restrict yHistory, const sample &x) {

    using Coefficients = iir::CoefficientsGetter<coeff, Filter>;

    sample Y = 0;
    sample X = x; // input is xN0
    sample yN0 = 0.0;
    size_t order = Coefficients::getOrder(filter);
    size_t i, j;
    for (i = 0, j = 1; i < order; i++, j++) {
      const sample xN1 = xHistory[i];
      const sample yN1 = yHistory[i];
      xHistory[i] = X;
      X = xN1;
      yHistory[i] = Y;
      Y = yN1;
      const coeff &coeffX = Coefficients::getX(j, filter);
      const coeff &coeffY = Coefficients::getY(j, filter);
      yN0 += xN1 * coeffX + yN1 * coeffY;
    }
    yN0 += Coefficients::getX(0) * x;

    yHistory[0] = yN0;

    return yN0;
  }

  /**
   * Calculates the next output value for the input value @code{x} with a fixed
   * filter order.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @tparam ORDER The filter order that must be equal to that of the
   * coefficients
   * @param filter The filter coefficients
   * @param xHistory State: the previous input values (at least as much as the
   * order)
   * @param yHistory State: the previous output values (at least as much as the
   * order)
   * @param x the input value
   * @return the calculated output value
   */
  template <class Filter, typename sample, size_t ORDER>
  sdsp_nodiscard sdsp_force_inline static sample
  withHistoryFixedOrder(const Filter &filter, sample *__restrict xHistory,
                        sample *__restrict yHistory, const sample &x) {

    using Coefficients = iir::CoefficientsGetter<coeff, Filter>;
    static_assert(ORDER == Coefficients::getOrder(filter),
                  "IIRFilterBase::withHistoryFixedOrder: specified order must "
                  "be same as coefficients' "
                  "order.");

    sample Y = 0;
    sample X = x; // input is xN0
    sample yN0 = Coefficients::getX(0) * x;
    size_t i, j;
    for (i = 0, j = 1; i < ORDER; i++, j++) {
      const sample xN1 = xHistory[i];
      const sample yN1 = yHistory[i];
      xHistory[i] = X;
      X = xN1;
      yHistory[i] = Y;
      Y = yN1;
      const coeff &coeffX = Coefficients::getX(j, filter);
      const coeff &coeffY = Coefficients::getY(j, filter);
      yN0 += xN1 * coeffX + yN1 * coeffY;
    }

    yHistory[0] = yN0;

    return yN0;
  }

  /**
   * Calculates the output values for input buffer @code{x} into output buffer
   * @code{y}.
   *
   * The result is calculated with all input and output values in the past as
   * zero.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @param filter The filter coefficients
   * @param x input samples
   * @param y output samples
   * @param count the number of samples to calculate
   */
  template <class Filter, typename sample>
  sdsp_force_inline static void
  withBuffersNoPast(const Filter &filter, const sample *__restrict x,
                    sample *__restrict y, size_t count) {

    using Coefficients = iir::CoefficientsGetter<sample, Filter>;
    const size_t order = Coefficients::getOrder(filter);

    for (size_t n = 0; n < count; n++) {
      sample yN = Coefficients::getX(0, filter) * x[n];
      ptrdiff_t h = n - 1;
      for (size_t j = 1; h >= 0 && j <= order; j++, h--) {
        const coeff &coeffX = Coefficients::getX(j, filter);
        const coeff &coeffY = Coefficients::getY(j, filter);
        yN += x[h] * coeffX + y[h] * coeffY;
      }
      y[n] = yN;
    }
  }

  /**
   * Calculates the output values for input buffer @code{x} into output buffer
   * @code{y}.
   *
   * Calculation starts at the offset and past values of the first samples are
   * taken from before the offset. This means that the offset should be equal to
   * or larger than the order and the buffer sizes should be at least
   * @code{offset + count} samples.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @param filter The filter coefficients
   * @param x input samples
   * @param y output samples
   * @param offset the number of samples to skip from the start, that must be at
   * least as large as the order.
   * @param count the number of samples to calculate
   * @return false if the offset is not equal to or larger than the order
   */
  template <class Filter, typename sample>
  sdsp_nodiscard sdsp_force_inline static bool
  withBuffers(const Filter &filter, sample *__restrict x, sample *__restrict y,
              size_t offset, size_t count) {

    using Coefficients = iir::CoefficientsGetter<sample, Filter>;
    const size_t order = Coefficients::getOrder(filter);
    if (offset < order) {
      return false;
    }

    for (size_t n = offset; n < count; n++) {
      sample yN = Coefficients::getX(0, filter) * x[n];
      for (size_t j = 1; j <= order; j++) {
        const coeff &coeffX = Coefficients::getX(j, filter);
        const coeff &coeffY = Coefficients::getY(j, filter);
        yN += x[n - j] * coeffX + y[n - j] * coeffY;
      }
      y[n] = yN;
    }
    return true;
  }

  /**
   * Calculates the output values for input buffer @code{x} into output buffer
   * @code{y}.
   *
   * Calculation starts at @code{OFFSET} and past values of the first samples
   * are taken from before the offset. This means that the offset must be equal
   * to or larger than
   * @coce{ORDER} and the buffer sizes should be at least @code{ORDER + count}
   * samples.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @tparam ORDER The filter order
   * @tparam OFFSET The offset of the first sample
   * @param filter The filter coefficients that must be of the same order as
   * @code{ORDER}
   * @param x input samples
   * @param y output samples
   * @param count The number of samples to calculate
   */
  template <class Filter, typename sample, size_t ORDER, size_t OFFSET>
  sdsp_force_inline static void
  withBuffersFixedOrderOffset(const Filter &filter, sample *__restrict x,
                              sample *__restrict y, size_t count) {

    static_assert(OFFSET < ORDER,
                  "IIRFilterBase::withBuffersFixedOrderOffset: "
                  "OFFSET must be equal to or larger than ORDER.");
    using Coefficients = iir::CoefficientsGetter<sample, Filter>;
    static_assert(ORDER == Coefficients::getOrder(filter),
                  "IIRFilterBase::withHistoryFixedOrder: specified order must "
                  "be same as coefficients' "
                  "order.");

    for (size_t n = OFFSET; n < count; n++) {
      sample yN = Coefficients::getX(0, filter) * x[n];
      for (size_t j = 1; j <= ORDER; j++) {
        const coeff &coeffX = Coefficients::getX(j, filter);
        const coeff &coeffY = Coefficients::getY(j, filter);
        yN += x[n - j] * coeffX + y[n - j] * coeffY;
      }
      y[n] = yN;
    }
  }

  /**
   * Calculates the output values for input buffer @code{x} into output buffer
   * @code{y}.
   *
   * Calculation starts at @code{OFFSET} and past values of the first samples
   * are taken from before the offset. This means that the offset must be equal
   * to or larger than
   * @coce{ORDER} and the buffer sizes should be at least @code{ORDER + count}
   * samples.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @tparam ORDER The filter order
   * @tparam OFFSET The offset of the first sample
   * @tparam COUNT The number of samples to calculate
   * @param filter The filter coefficients that must be of the same order as
   * @code{ORDER}
   * @param x input samples
   * @param y output samples
   */
  template <class Filter, typename sample, size_t ORDER, size_t OFFSET,
            size_t COUNT>
  sdsp_force_inline static void
  withBuffersFixedOrderOffsetCount(const Filter &filter, sample *__restrict x,
                                   sample *__restrict y) {

    static_assert(OFFSET < ORDER,
                  "IIRFilterBase::withBuffersFixedOrderOffsetCount: "
                  "OFFSET must be equal to or larger than ORDER.");
    using Coefficients = iir::CoefficientsGetter<sample, Filter>;
    static_assert(ORDER == Coefficients::getOrder(filter),
                  "IIRFilterBase::withHistoryFixedOrder: specified order must "
                  "be same as coefficients' order.");

    for (size_t n = OFFSET; n < COUNT; n++) {
      sample yN = Coefficients::getX(0, filter) * x[n];
      for (size_t j = 1; j <= ORDER; j++) {
        const coeff &coeffX = Coefficients::getX(j, filter);
        const coeff &coeffY = Coefficients::getY(j, filter);
        yN += x[n - j] * coeffX + y[n - j] * coeffY;
      }
      y[n] = yN;
    }
  }

  /**
   * Calculates the output values for input buffer @code{x} into output buffer
   * @code{y}, backwards in time, starting from sample @code{count - 1}
   * downwards to zero.
   *
   * The result is calculated with all input and output values in the past as
   * zero.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @param filter The filter coefficients
   * @param x input samples
   * @param y output samples
   * @param count the number of samples to calculate
   */
  template <class Filter, typename sample>
  sdsp_force_inline static void
  withBuffersBackwardsNoPast(const Filter &filter, const sample *__restrict x,
                             sample *__restrict y, size_t count) {

    using Coefficients = iir::CoefficientsGetter<sample, Filter>;
    const size_t order = filter.order;

    for (ptrdiff_t n = count - 1; n >= 0; n--) {
      sample yN0 = Coefficients::getX(0) * x[n];
      for (ptrdiff_t j = 0, h = n + 1; h < count && j < order; j++, h++) {
        const coeff &coeffX = Coefficients::getX(j + 1);
        const coeff &coeffY = Coefficients::getY(j);
        yN0 += x[h] * coeffX + y[h] * coeffY;
      }
      y[n] = yN0;
    }
  }

  /**
   * Calculates the output values for input buffer @code{x} into output buffer
   * @code{y}, backwards in time, starting from sample @code{count - 1}
   * downwards to zero.
   *
   * Calculation starts at the @code{count - 1} but uses "past" input and output
   * samples that are at offsets @code{count - 1 + order}, so the buffer sizes
   * must be at least
   * @code{count + order} samples.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @param filter The filter coefficients
   * @param x input samples
   * @param y output samples
   * @param count the number of samples to calculate
   */
  template <class Filter, typename sample>
  sdsp_force_inline static void
  withBuffersBackwards(const Filter &filter, sample *__restrict x,
                       sample *__restrict y, size_t count) {

    using Coefficients = iir::CoefficientsGetter<sample, Filter>;
    const size_t order = Coefficients::getOrder(filter);

    for (ptrdiff_t n = count - 1; n >= 0; n--) {
      sample yN = Coefficients::getX(0, filter) * x[n];
      for (size_t j = 1; j <= order; j++) {
        const coeff &coeffX = Coefficients::getX(j, filter);
        const coeff &coeffY = Coefficients::getY(j, filter);
        yN += x[n + j] * coeffX + y[n + j] * coeffY;
      }
      y[n] = yN;
    }
  }

  /**
   * Calculates the output values for input buffer @code{x} into output buffer
   * @code{y}, backwards in time, starting from sample @code{count - 1}
   * downwards to zero.
   *
   * Calculation starts at the @code{count - 1} but uses "past" input and output
   * samples that are at offsets @code{count - 1 + ORDER}, so the buffer sizes
   * must be at least
   * @code{count + ORDER} samples.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @tparam ORDER The filter order
   * @param filter The filter coefficients
   * @param x input samples
   * @param y output samples
   * @param count the number of samples to calculate
   */
  template <class Filter, typename sample, size_t ORDER>
  sdsp_force_inline static void
  withBuffersBackwardsFixedOrder(const Filter &filter, sample *__restrict x,
                                 sample *__restrict y, size_t count) {

    using Coefficients = iir::CoefficientsGetter<sample, Filter>;
    static_assert(ORDER == Coefficients::getOrder(filter),
                  "IIRFilterBase::withBuffersBackwardsFixedOrder: specified "
                  "order must be same as "
                  "coefficients' order.");

    for (ptrdiff_t n = count - 1; n >= 0; n--) {
      sample yN = Coefficients::getX(0, filter) * x[n];
      for (size_t j = 1; j <= ORDER; j++) {
        const coeff &coeffX = Coefficients::getX(j, filter);
        const coeff &coeffY = Coefficients::getY(j, filter);
        yN += x[n + j] * coeffX + y[n + j] * coeffY;
      }
      y[n] = yN;
    }
  }

  /**
   * Calculates the output values for input buffer @code{x} into output buffer
   * @code{y}, backwards in time, starting from sample @code{count - 1}
   * downwards to zero.
   *
   * Calculation starts at the @code{count - 1} but uses "past" input and output
   * samples that are at offsets @code{count - 1 + ORDER}, so the buffer sizes
   * must be at least
   * @code{count + ORDER} samples.
   *
   * @tparam Filter Filter coefficients class
   * @tparam sample The type of samples
   * @tparam ORDER The filter order
   * @tparam COUNT the number of samples to calculate
   * @param filter The filter coefficients
   * @param x input samples
   * @param y output samples
   */
  template <class Filter, typename sample, size_t ORDER, size_t COUNT>
  sdsp_force_inline static void withBuffersBackwardsFixedOrderCount(
      const Filter &filter, sample *__restrict x, sample *__restrict y) {

    using Coefficients = iir::CoefficientsGetter<sample, Filter>;
    static_assert(ORDER == Coefficients::getOrder(filter),
                  "IIRFilterBase::withBuffersBackwardsFixedOrderCount: "
                  "specified order must be same as "
                  "coefficients' order.");

    for (ptrdiff_t n = COUNT - 1; n >= 0; n--) {
      sample yN = Coefficients::getX(0, filter) * x[n];
      for (size_t j = 1; j <= ORDER; j++) {
        const coeff &coeffX = Coefficients::getX(j, filter);
        const coeff &coeffY = Coefficients::getY(j, filter);
        yN += x[n + j] * coeffX + y[n + j] * coeffY;
      }
      y[n] = yN;
    }
  }
};

/**
 * Provides methods to "wrap" history data for buffer based filtering that uses
 * history.
 *
 * The IIRFiltebase methods that also use past data, assume history
 * "surrounding" those buffers. If the same input and output buffers are used
 * for the next block of samples, glitches can only be prevented when the the
 * lastly generated data is "wrapped" so that it becomes the history for the
 * next blockof samples.
 *
 * The parameters of these methods are the same as those of the corresponding
 * filter methods.
 *
 * The methods are force-inline and should be used to construct methods with
 * specific array indexed access.
 */
struct BufferHistoryWrap {

  template <class C>
  sdsp_force_inline static void forwardUnchecked(C &data, size_t offset,
                                                 size_t order, size_t count) {
    for (size_t dst = offset - order, src = dst + count; dst < offset;
         ++dst, ++src) {
      data[dst] = data[src];
    }
  }

  template <class C>
  sdsp_force_inline static void withBuffers(C &data, size_t offset,
                                            size_t order, size_t count) {
    if (offset < order) {
      throw std::invalid_argument("BufferHistoryWrap:forward(): "
                                  "offset must be greater than order.");
    }
    forwardUnchecked(data, offset, order, count);
  }

  template <class C, size_t order>
  sdsp_force_inline static void withBuffersFixedOrder(C &data, size_t offset,
                                                      size_t count) {
    static_assert(offset >= order, "BufferHistoryWrap:forward(): "
                                   "offset must be greater than order.");
    for (size_t dst = offset - order, src = dst + count; dst < offset;
         ++dst, ++src) {
      data[dst] = data[src];
    }
  }

  template <class C, size_t offset, size_t order>
  sdsp_force_inline static void withBuffersFixedOffsetOrder(C &data,
                                                            size_t count) {
    static_assert(offset >= order, "BufferHistoryWrap:forward(): "
                                   "offset must be greater than order.");
    for (size_t dst = offset - order, src = dst + count; dst < offset;
         ++dst, ++src) {
      data[dst] = data[src];
    }
  }

  template <class C, size_t offset, size_t order, size_t count>
  sdsp_force_inline static void withBuffersFixedOffsetOrderCount(C &data) {
    static_assert(offset >= order, "BufferHistoryWrap:forward(): "
                                   "offset must be greater than order.");
    for (size_t dst = offset - order, src = dst + count; dst < offset;
         ++dst, ++src) {
      data[dst] = data[src];
    }
  }

  template <class C>
  sdsp_force_inline static void withBuffersBackwards(C &data, size_t order,
                                                     size_t count) {
    for (size_t src = order - 1, dst = src + count; dst >= count;
         dst--, src--) {
      data[dst] = data[src];
    }
  }

  template <class C, size_t ORDER>
  sdsp_force_inline static void withBuffersBackwardsFixedOrder(C &data,
                                                               size_t count) {
    for (size_t src = ORDER - 1, dst = src + count; dst >= count;
         dst--, src--) {
      data[dst] = data[src];
    }
  }

  template <class C, size_t ORDER, size_t COUNT>
  sdsp_force_inline static void withBuffersBackwardsFixedOrderCount(C &data) {
    for (size_t src = ORDER - 1, dst = src + COUNT; dst >= COUNT;
         dst--, src--) {
      data[dst] = data[src];
    }
  }
};

template <typename coeff, size_t ORDER>
struct IIRFixedOrderCoefficients : public IIRCoefficientsClass {

  class Setter : public iir::CoefficientsSetter {
    explicit Setter(IIRFixedOrderCoefficients &o) : owner(o) {}
    Setter(Setter &&source) noexcept : owner(source.owner) {}

    sdsp_nodiscard unsigned getOrder() const override { return ORDER; }
    sdsp_nodiscard unsigned getMaxOrder() const override { return ORDER; }
    sdsp_nodiscard bool canSetOrder() const override { return false; }

  protected:
    void setValidOrder(unsigned) override {
      throw std::runtime_error(
          "IIRFixedOrderCoefficients::setValidOrder(): illegal call");
    }
    void setValidX(size_t i, double value) override { owner.c[i] = value; }
    void setValidY(size_t i, double value) override { owner.d[i] = value; }

  private:
    IIRFixedOrderCoefficients &owner;
  };

  Setter setter() { return Setter(*this); }

  explicit IIRFixedOrderCoefficients(const IIRFixedOrderCoefficients &source) {
    operator=(source);
  }

  IIRFixedOrderCoefficients(IIRFixedOrderCoefficients &&source) noexcept {
    operator=(source);
  }

  template <class Coefficients>
  explicit IIRFixedOrderCoefficients(const Coefficients &source) {
    operator=(source);
  }

  explicit IIRFixedOrderCoefficients(double scale) { setter().unity(scale); }

  IIRFixedOrderCoefficients &
  operator=(const IIRFixedOrderCoefficients &source) {
    for (size_t i = 0; i <= ORDER; i++) {
      c[i] = source.c[i];
      d[i] = source.d[i];
    }
    return *this;
  }

  template <class Coefficients>
  IIRFixedOrderCoefficients &operator=(const Coefficients &source) {
    using Access = iir::CoefficientsGetter<coeff, Coefficients>;
    if (Access::getOrder(source) != ORDER) {
      throw std::invalid_argument(
          "IIRFixedOrderCoefficients::operator=(source): "
          "source must have same order.");
    }
    for (size_t i = 0; i <= ORDER; i++) {
      c[i] = Access::getX(i, source);
      d[i] = Access::getY(i, source);
    }
    return *this;
  }

  sdsp_nodiscard sdsp_force_inline coeff getC(size_t i) const { return c[i]; }
  sdsp_nodiscard sdsp_force_inline coeff getD(size_t i) const { return d[i]; }
  sdsp_nodiscard sdsp_force_inline constexpr size_t getOrder() const {
    return ORDER;
  }
  sdsp_nodiscard sdsp_force_inline constexpr size_t getMaxOrder() const {
    return ORDER;
  }

private:
  friend class Setter;
  coeff c[ORDER + 1];
  coeff d[ORDER + 1];
};

template <typename coeff> struct IIRCoefficients : public IIRCoefficientsClass {

  struct Setter : public iir::CoefficientsSetter {
    explicit Setter(IIRCoefficients &o) : owner(o) {}
    Setter(Setter &&source) noexcept : owner(source.owner) {}
    sdsp_nodiscard unsigned getOrder() const override {
      return owner.getOrder();
    }
    sdsp_nodiscard unsigned getMaxOrder() const override {
      return owner.getMaxOrder();
    }
    sdsp_nodiscard bool canSetOrder() const override { return true; }

  protected:
    void setValidOrder(unsigned newOrder) override { owner.setOrder(newOrder); }
    void setValidX(size_t i, double value) override { owner.c[i] = value; }
    void setValidY(size_t i, double value) override { owner.d[i] = value; }

  private:
    IIRCoefficients &owner;
  };

public:
  Setter setter() { return Setter(*this); }

  IIRCoefficients(size_t maximumOrder, size_t initialOrder)
      : c(new coeff[allocationSize(maximumOrder)]), maxOrder(maximumOrder),
        d(c + maximumOrder + 1), order(validOrder(initialOrder, maxOrder)) {}

  explicit IIRCoefficients(size_t order) : IIRCoefficients(order, order) {}

  explicit IIRCoefficients(const IIRCoefficients &source)
      : IIRCoefficients(source.getMaxOrder(), source.getOrder()) {
    operator=(source);
  }

  IIRCoefficients(IIRCoefficients &&source) noexcept
      : IIRCoefficients(source.getMaxOrder(), source.getOrder()) {
    operator=(source);
  }

  explicit IIRCoefficients(size_t order, double scale)
      : IIRCoefficients(order, order) {
    setter().unity(scale);
  }

  IIRCoefficients &operator=(const IIRCoefficients &source) {
    size_t sourceOrder = source.getOrder();
    if (sourceOrder != order) {
      if (!isValidOrder(sourceOrder, maxOrder)) {
        throw std::invalid_argument("IIRCoefficients::operator=(source): "
                                    "cannot adopt source order (invalid).");
      }
    }
    order = sourceOrder;
    for (size_t i = 0; i <= order; i++) {
      c[i] = source.c[i];
      d[i] = source.d[i];
    }
    return *this;
  }

  template <class Coefficients>
  IIRCoefficients &operator=(const Coefficients &source) {
    using Access = iir::CoefficientsGetter<coeff, Coefficients>;
    size_t sourceOrder = Access::getOrder(source);
    if (sourceOrder != order) {
      if (!isValidOrder(sourceOrder, maxOrder)) {
        throw std::invalid_argument("IIRCoefficients::operator=(source): "
                                    "cannot adopt source order (invalid).");
      }
    }
    order = sourceOrder;
    for (size_t i = 0; i <= order; i++) {
      c[i] = Access::getX(i, source);
      d[i] = Access::getY(i, source);
    }
    return *this;
  }

  sdsp_nodiscard sdsp_force_inline coeff getC(size_t i) const { return c[i]; }
  sdsp_nodiscard sdsp_force_inline coeff getD(size_t i) const { return d[i]; }
  sdsp_nodiscard sdsp_force_inline size_t getOrder() const { return order; }
  sdsp_nodiscard sdsp_force_inline size_t getMaxOrder() const {
    return maxOrder;
  }

  ~IIRCoefficients() {
    delete[] c;
    c = d = nullptr;
    maxOrder = order = 0;
  }

protected:
  void setOrder(size_t newOrder) { order = validOrder(newOrder, maxOrder); }

private:
  static size_t allocationSize(size_t order) {
    return (iir::get_valid_order(order) * 1) * 2;
  }

  static size_t validOrder(size_t order, size_t maxOrder) {
    if (isValidOrder(order, maxOrder)) {
      return order;
    }
    throw std::invalid_argument(
        "IIRFixedOrderCoefficients::validOrder(order, maxOrder):"
        "order must be positive and cannot exceed maxOrder.");
  }
  static bool isValidOrder(size_t order, size_t maxOrder) {
    return order > 0 && order <= maxOrder;
  }

  coeff *c;
  size_t maxOrder;
  coeff *d;
  size_t order;
};

} // namespace simpledsp

#endif // SIMPLE_DSP_IIR_H
