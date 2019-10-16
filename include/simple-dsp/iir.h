#ifndef SIMPLE_DSP_IIR_H
#define SIMPLE_DSP_IIR_H
/*
 * simple-dsp/iir.h
 *
 * Added by michel on 2019-10-13
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

#include <cstddef>
#include <simple-dsp/addressing.h>
#include <simple-dsp/attributes.h>

namespace simpledsp {

  /**
   * Allows algorithms that calculate filter coefficients to set them in a coefficient
   * implementation. The implementation can remain highly optimised but setting the calculated
   * coefficients can be done in a standard way.
   *
   * @tparam T type of coefficent values
   */
  struct IIRCoefficientsSetter {
    using Check = IndexPolicyBase<size_t, IndexPolicyType::THROW>;

    sdsp_nodiscard virtual unsigned getOrder() const = 0;
    sdsp_nodiscard virtual unsigned getMaxOrder() const = 0;
    sdsp_nodiscard virtual bool canSetOrder() const = 0;
    virtual ~IIRCoefficientsSetter() = default;

    sdsp_nodiscard unsigned getCoefficients() const { return getOrder() + 1; }

    IIRCoefficientsSetter& setOrder(unsigned order) {
      if (order == 0) {
          throw std::invalid_argument("IIRCoefficientsSetter::setOrder(order): "
                                      "order must be 1 or higher.");
      }
      if (!canSetOrder()) {
        if (order == getOrder()) {
          return *this;
        }
        throw std::runtime_error("IIRCoefficientsSetter::setOrder(order): "
                                 "implementation does not allow change of filter order.");
      }
      if (order > getMaxOrder()) {
        throw std::invalid_argument("IIRCoefficientsSetter::setOrder(order): "
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

    IIRCoefficientsSetter& setC(size_t i, double value) {
      setValidC(Check::index(i, getCoefficients()), value);
      return *this;
    }

    IIRCoefficientsSetter& setD(size_t i, double value) {
      setValidD(Check::index(i, getCoefficients()), value);
      return *this;
    }

  protected:
    virtual void setValidOrder(unsigned order) = 0;
    virtual void setValidC(size_t i, double value) = 0;
    virtual void setValidD(size_t i, double value) = 0;
  };

  /**
   * A tagging interface that contains no methods but is necessary to use a class
   * in an IIRCoefficientAccess wrapper.
   */
  struct IIRCoefficientsClass {};

  template<typename coeff, class CoefficientsClass>
  struct IIRCoefficientAccess {
    static_assert(
            std::is_base_of<IIRCoefficientsClass, CoefficientsClass>::value,
            "IIRCoefficientAccess<coeff, CoefficientsClass>: "
            "CoefficientsClass not derived from IIRCoefficientsClass");
    static_assert(std::is_floating_point<coeff>::value,
            "IIRCoefficientAccess<coeff, CoefficientsClass>: "
            "coeff must be floating-point");

    sdsp_nodiscard sdsp_force_inline static const coeff &getC(size_t i, const CoefficientsClass
    &coeffs) {
      return coeffs.getC(i);
    }
    sdsp_nodiscard sdsp_force_inline static const coeff &getD(size_t i, const CoefficientsClass &coeffs) {
      return coeffs.getD(i);
    }
    sdsp_nodiscard sdsp_force_inline static constexpr unsigned getOrder(const CoefficientsClass &coeffs) {
      return coeffs.getOrder();
    }
    sdsp_nodiscard sdsp_force_inline static constexpr unsigned getMaxOrder(const CoefficientsClass &coeffs) {
      return coeffs.getMaxOrder();
    }
  };

  /**
   * Defines how an iteration of an IIR claculation is done. Some algorithms specify
   * coefficients where output history values (y) are added to the result and others expect them
   * to be subtracted.
   */
  enum class IIRCalculationMethod { POSITIVE_Y, NEGATIVE_Y };
  static constexpr IIRCalculationMethod IIR_CALCULATION_METHOD_DEFAULT =
          IIRCalculationMethod::POSITIVE_Y;

  template<typename coeff, class sample, IIRCalculationMethod method>
  struct IIRSingleCalculation;

  template<typename coeff, class sample>
  struct IIRSingleCalculation<coeff, sample, IIRCalculationMethod::NEGATIVE_Y> {
    sdsp_force_inline static coeff iterate(
            const coeff &coeffX, const sample& x, const coeff &coeffY, const sample&y) {
      return x * coeffX - y * coeffY;
    }
  };

  template<typename coeff, class sample>
  struct IIRSingleCalculation<coeff, sample, IIRCalculationMethod::POSITIVE_Y> {
    sdsp_force_inline static coeff iterate(
            const coeff &coeffX, const sample& x, const coeff &coeffY, const sample&y) {
      return x * coeffX + y * coeffY;
    }
  };

  /**
   * Using the coefficient parameter and method of calculation, delivers various ways to
   * calculate IIR filters.
   * @tparam coeff The coefficient value type
   * @tparam method The way to calculate the filter with given coefficients
   */
  template<typename coeff, IIRCalculationMethod method = IIR_CALCULATION_METHOD_DEFAULT>
  struct IIRFilterBase {

    /**
     * Calculates the next output value for the input value @code{x}.
     *
     * @tparam Filter Filter coefficients class
     * @tparam sample The type of samples
     * @param filter The filter coefficients
     * @param xHistory State: the previous input values (at least as much as the order)
     * @param yHistory State: the previous output values (at least as much as the order)
     * @param x the input value
     * @return the calculated output value
     */
    template<class Filter, typename sample>
    sdsp_nodiscard sdsp_force_inline static sample withHistory(
            const Filter &filter,
            sample* __restrict xHistory,
            sample* __restrict yHistory,
            const sample& x) {

      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<coeff, Filter>;

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
        yN0 += Calculation::iterate(
                Coefficients::getC(j, filter), xN1,
                Coefficients::getD(j, filter), yN1);
      }
      yN0 += Coefficients::getC(0) * x;

      yHistory[0] = yN0;

      return yN0;
    }

    /**
     * Calculates the next output value for the input value @code{x} with a fixed filter order.
     *
     * @tparam Filter Filter coefficients class
     * @tparam sample The type of samples
     * @tparam ORDER The filter order that must be equal to that of the coefficients
     * @param filter The filter coefficients
     * @param xHistory State: the previous input values (at least as much as the order)
     * @param yHistory State: the previous output values (at least as much as the order)
     * @param x the input value
     * @return the calculated output value
     */
    template<class Filter, typename sample, size_t ORDER>
    sdsp_nodiscard sdsp_force_inline static sample withHistoryFixedOrder(
            const Filter &filter,
            sample* __restrict xHistory,
            sample* __restrict yHistory,
            const sample& x) {

      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<coeff, Filter>;
      static_assert(
              ORDER == Coefficients::getOrder(filter),
              "IIRFilterBase::withHistoryFixedOrder: specified order must be same as coefficients' "
              "order.");

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
        yN0 += Calculation::iterate(
                Coefficients::getC(j, filter), xN1,
                Coefficients::getD(j, filter), yN1);
      }
      yN0 += Coefficients::getC(0) * x;

      yHistory[0] = yN0;

      return yN0;
    }

    /**
     * Calculates the output values for input buffer @code{x} into output buffer @code{y}.
     *
     * The result is calculated with all input and output values in the past as zero.
     *
     * @tparam Filter Filter coefficients class
     * @tparam sample The type of samples
     * @param filter The filter coefficients
     * @param x input samples
     * @param y output samples
     * @param count the number of samples to calculate
     */
    template<class Filter, typename sample>
    sdsp_force_inline static void withBuffersNoPast(
            const Filter &filter, const sample * __restrict x, sample * __restrict y, size_t count) {

      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<sample, Filter>;
      const size_t order = Coefficients::getOrder(filter);

      for (size_t n = 0; n < count; n++) {
        sample yN = Coefficients::getC(0, filter) * x[n];
        ptrdiff_t h = n - 1;
        for (size_t j = 1; h >= 0 && j <= order; j++, h--) {
          yN += Calculation::iterate(
                  Coefficients::getC(j, filter), x[h],
                  Coefficients::getD(j, filter), y[h]);
        }
        y[n] = yN;
      }
    }

    /**
     * Calculates the output values for input buffer @code{x} into output buffer @code{y}.
     *
     * Calculation starts at the offset and past values of the first samples are taken from
     * before the offset. This means that the offset should be equal to or larger than the
     * order and the buffer sizes should be at least @code{offset + count} samples.
     *
     * @tparam Filter Filter coefficients class
     * @tparam sample The type of samples
     * @param filter The filter coefficients
     * @param x input samples
     * @param y output samples
     * @param offset the number of samples to skip from the start, that must be at least as large
     * as the order.
     * @param count the number of samples to calculate
     * @return false if the offset is not equal to or larger than the order
     */
    template<class Filter, typename sample>
    sdsp_nodiscard sdsp_force_inline static bool withBuffers(
            const Filter &filter, sample * __restrict x, sample * __restrict y,
            size_t offset, size_t count) {

      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<sample, Filter>;
      const size_t order = Coefficients::getOrder(filter);
      if (offset < order) {
        return false;
      }

      for (size_t n = offset; n < count; n++) {
        sample yN = Coefficients::getC(0, filter) * x[n];
        for (size_t j = 1; j <= order; j++) {
          yN += Calculation::iterate(
                  Coefficients::getC(j, filter), x[n - j],
                  Coefficients::getD(j, filter), y[n - j]);
        }
        y[n] = yN;
      }
      for (size_t dst = offset - order, src = dst + count; dst < offset; dst++, src++) {
        x[dst] = x[src];
        y[dst] = y[src];
      }

      return true;
    }

    /**
     * Calculates the output values for input buffer @code{x} into output buffer @code{y}.
     *
     * Calculation starts at @code{OFFSET} and past values of the first samples are taken
     * from before the offset. This means that the offset must be equal to or larger than
     * @coce{ORDER} and the buffer sizes should be at least @code{ORDER + count} samples.
     *
     * @tparam Filter Filter coefficients class
     * @tparam sample The type of samples
     * @tparam ORDER The filter order
     * @tparam OFFSET The offset of the first sample
     * @param filter The filter coefficients that must be of the same order as @code{ORDER}
     * @param x input samples
     * @param y output samples
     * @param count The number of samples to calculate
     */
    template<class Filter, typename sample, size_t ORDER, size_t OFFSET>
    sdsp_force_inline static void withBuffersFixedOrderOffset(
            const Filter &filter, sample * __restrict x, sample * __restrict y,
            size_t count) {

      static_assert(OFFSET < ORDER, "IIRFilterBase::withBuffersFixedOrderOffset: "
                                    "OFFSET must be equal to or larger than ORDER.");
      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<sample, Filter>;
      static_assert(
              ORDER == Coefficients::getOrder(filter),
              "IIRFilterBase::withHistoryFixedOrder: specified order must be same as coefficients' "
              "order.");

      for (size_t n = OFFSET; n < count; n++) {
        sample yN = Coefficients::getC(0, filter) * x[n];
        for (size_t j = 1; j <= ORDER; j++) {
          yN += Calculation::iterate(
                  Coefficients::getC(j, filter), x[n - j],
                  Coefficients::getD(j, filter), y[n - j]);
        }
        y[n] = yN;
      }
      for (size_t dst = OFFSET - ORDER, src = dst + count; dst < OFFSET; dst++, src++) {
        x[dst] = x[src];
        y[dst] = y[src];
      }
    }

    /**
     * Calculates the output values for input buffer @code{x} into output buffer @code{y}.
     *
     * Calculation starts at @code{OFFSET} and past values of the first samples are taken
     * from before the offset. This means that the offset must be equal to or larger than
     * @coce{ORDER} and the buffer sizes should be at least @code{ORDER + count} samples.
     *
     * @tparam Filter Filter coefficients class
     * @tparam sample The type of samples
     * @tparam ORDER The filter order
     * @tparam OFFSET The offset of the first sample
     * @tparam COUNT The number of samples to calculate
     * @param filter The filter coefficients that must be of the same order as @code{ORDER}
     * @param x input samples
     * @param y output samples
     */
    template<class Filter, typename sample, size_t ORDER, size_t OFFSET, size_t COUNT>
    sdsp_force_inline static void withBuffersFixedOrderOffsetCount(
            const Filter &filter, sample * __restrict x, sample * __restrict y) {

      static_assert(OFFSET < ORDER, "IIRFilterBase::withBuffersFixedOrderOffsetCount: "
                                    "OFFSET must be equal to or larger than ORDER.");
      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<sample, Filter>;
      static_assert(
              ORDER == Coefficients::getOrder(filter),
              "IIRFilterBase::withHistoryFixedOrder: specified order must be same as coefficients' "
              "order.");

      for (size_t n = OFFSET; n < COUNT; n++) {
        sample yN = Coefficients::getC(0, filter) * x[n];
        for (size_t j = 1; j <= ORDER; j++) {
          yN += Calculation::iterate(
                  Coefficients::getC(j, filter), x[n - j],
                  Coefficients::getD(j, filter), y[n - j]);
        }
        y[n] = yN;
      }
      for (size_t dst = OFFSET - ORDER, src = dst + COUNT; dst < OFFSET; dst++, src++) {
        x[dst] = x[src];
        y[dst] = y[src];
      }
    }

    /**
     * Calculates the output values for input buffer @code{x} into output buffer @code{y},
     * backwards in time, starting from sample @code{count - 1} downwards to zero.
     *
     * The result is calculated with all input and output values in the past as zero.
     *
     * @tparam Filter Filter coefficients class
     * @tparam sample The type of samples
     * @param filter The filter coefficients
     * @param x input samples
     * @param y output samples
     * @param count the number of samples to calculate
     */
    template<class Filter, typename sample>
    sdsp_force_inline static void withBuffersBackwardsNoPast(
            const Filter &filter, const sample * __restrict x, sample * __restrict y, size_t count) {

      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<sample, Filter>;
      const size_t order = filter.order;

      for (ptrdiff_t n = count - 1; n >= 0; n--) {
        sample yN0 = Coefficients::getC(0) * x[n];
        for (ptrdiff_t j = 0, h = n + 1; h < count && j < order; j++, h++) {
          yN0 += Calculation::iterate(Coefficients::getC(j + 1), x[h], Coefficients::getD(j), y[h]);
        }
        y[n] = yN0;
      }
    }

    /**
     * Calculates the output values for input buffer @code{x} into output buffer @code{y},
     * backwards in time, starting from sample @code{count - 1} downwards to zero.
     *
     * Calculation starts at the @code{count - 1} but uses "past" input and output samples that
     * are at offsets @code{count - 1 + order}, so the buffer sizes must be at least
     * @code{count + order} samples.
     *
     * @tparam Filter Filter coefficients class
     * @tparam sample The type of samples
     * @param filter The filter coefficients
     * @param x input samples
     * @param y output samples
     * @param count the number of samples to calculate
     */
    template<class Filter, typename sample>
    sdsp_force_inline static void withBuffersBackwards(
            const Filter &filter, sample * __restrict x, sample * __restrict y,
            size_t count) {

      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<sample, Filter>;
      const size_t order = Coefficients::getOrder(filter);

      for (ptrdiff_t n = count - 1; n >= 0; n--) {
        sample yN = Coefficients::getC(0, filter) * x[n];
        for (size_t j = 1; j <= order; j++) {
          yN += Calculation::iterate(
                  Coefficients::getC(j, filter), x[n + j],
                  Coefficients::getD(j, filter), y[n + j]);
        }
        y[n] = yN;
      }
      for (size_t src = 0, dst = count; dst < order; dst++, src++) {
        x[dst] = x[src];
        y[dst] = y[src];
      }
    }

    /**
     * Calculates the output values for input buffer @code{x} into output buffer @code{y},
     * backwards in time, starting from sample @code{count - 1} downwards to zero.
     *
     * Calculation starts at the @code{count - 1} but uses "past" input and output samples that
     * are at offsets @code{count - 1 + ORDER}, so the buffer sizes must be at least
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
    template<class Filter, typename sample, size_t ORDER>
    sdsp_force_inline static void withBuffersBackwardsFixedOrder(
            const Filter &filter, sample * __restrict x, sample * __restrict y,
            size_t count) {

      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<sample, Filter>;
      static_assert(
              ORDER == Coefficients::getOrder(filter),
              "IIRFilterBase::withBuffersBackwardsFixedOrder: specified order must be same as "
              "coefficients' order.");

      for (ptrdiff_t n = count - 1; n >= 0; n--) {
        sample yN = Coefficients::getC(0, filter) * x[n];
        for (size_t j = 1; j <= ORDER; j++) {
          yN += Calculation::iterate(
                  Coefficients::getC(j, filter), x[n + j],
                  Coefficients::getD(j, filter), y[n + j]);
        }
        y[n] = yN;
      }
      for (size_t src = 0, dst = count; dst < ORDER; dst++, src++) {
        x[dst] = x[src];
        y[dst] = y[src];
      }
    }

    /**
     * Calculates the output values for input buffer @code{x} into output buffer @code{y},
     * backwards in time, starting from sample @code{count - 1} downwards to zero.
     *
     * Calculation starts at the @code{count - 1} but uses "past" input and output samples that
     * are at offsets @code{count - 1 + ORDER}, so the buffer sizes must be at least
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
    template<class Filter, typename sample, size_t ORDER, size_t COUNT>
    sdsp_force_inline static void withBuffersBackwardsFixedOrderCount(
            const Filter &filter, sample * __restrict x, sample * __restrict y) {

      using Calculation = IIRSingleCalculation<coeff, sample, method>;
      using Coefficients = IIRCoefficientAccess<sample, Filter>;
      static_assert(
              ORDER == Coefficients::getOrder(filter),
              "IIRFilterBase::withBuffersBackwardsFixedOrderCount: specified order must be same as "
              "coefficients' order.");

      for (ptrdiff_t n = COUNT - 1; n >= 0; n--) {
        sample yN = Coefficients::getC(0, filter) * x[n];
        for (size_t j = 1; j <= ORDER; j++) {
          yN += Calculation::iterate(
                  Coefficients::getC(j, filter), x[n + j],
                  Coefficients::getD(j, filter), y[n + j]);
        }
        y[n] = yN;
      }
      for (size_t src = 0, dst = COUNT; dst < ORDER; dst++, src++) {
        x[dst] = x[src];
        y[dst] = y[src];
      }
    }

  };

  template<typename coeff, size_t ORDER>
  struct IIRFixedOrderCoefficients : public IIRCoefficientsClass {

    class Setter : public IIRCoefficientsSetter {
      explicit Setter(IIRFixedOrderCoefficients &o) : owner(o) {}
      Setter(Setter && source) noexcept : owner(source.owner) {}

      sdsp_nodiscard unsigned getOrder() const override { return ORDER;}
      sdsp_nodiscard unsigned getMaxOrder() const override { return ORDER; }
      sdsp_nodiscard bool canSetOrder() const override { return false; }

    protected:

      void setValidOrder(unsigned) override {
        throw std::runtime_error("IIRFixedOrderCoefficients::setValidOrder(): illegal call");
      }
      void setValidC(size_t i, double value) override { owner.c[i] = value; }
      void setValidD(size_t i, double value) override { owner.d[i] = value; }

    private:

      IIRFixedOrderCoefficients &owner;
    };

    Setter setter() {
      return Setter(*this);
    }

    explicit IIRFixedOrderCoefficients(const IIRFixedOrderCoefficients &source) {
      operator=(source);
    }

    IIRFixedOrderCoefficients(IIRFixedOrderCoefficients &&source) noexcept {
      operator=(source);
    }

    template<class Coefficients>
    explicit IIRFixedOrderCoefficients(const Coefficients &source) {
      operator=(source);
    }

    IIRFixedOrderCoefficients &operator = (const IIRFixedOrderCoefficients &source) {
      for (size_t i = 0; i <= ORDER; i++) {
        c[i] = source.c[i];
        d[i] = source.d[i];
      }
      return *this;
    }

    template<class Coefficients>
    IIRFixedOrderCoefficients &operator = (const Coefficients &source) {
      using Access = IIRCoefficientAccess<coeff, Coefficients>;
      if (Access::getOrder(source)  != ORDER) {
        throw std::invalid_argument("IIRFixedOrderCoefficients::operator=(source): "
                                    "source must have same order.");
      }
      for (size_t i = 0; i <= ORDER; i++) {
        c[i] = Access::getC(i, source);
        d[i] = Access::getD(i, source);
      }
      return *this;
    }

    sdsp_nodiscard sdsp_force_inline coeff getC(size_t i) const { return c[i]; }
    sdsp_nodiscard sdsp_force_inline coeff getD(size_t i) const { return d[i]; }
    sdsp_nodiscard sdsp_force_inline constexpr size_t getOrder() const { return ORDER; }
    sdsp_nodiscard sdsp_force_inline constexpr size_t getMaxOrder() const { return ORDER; }

  private:
    friend class Setter;
    coeff c[ORDER + 1];
    coeff d[ORDER + 1];
  };

  template<typename coeff>
  struct IIRCoefficients : public IIRCoefficientsClass {

    struct Setter : public IIRCoefficientsSetter {
      explicit Setter(IIRCoefficients &o) : owner(o) {}
      Setter(Setter && source) noexcept : owner(source.owner) {}
      sdsp_nodiscard unsigned getOrder() const override { return owner.getOrder();}
      sdsp_nodiscard unsigned getMaxOrder() const override { return owner.getMaxOrder(); }
      sdsp_nodiscard bool canSetOrder() const override { return true; }

    protected:

      void setValidOrder(unsigned newOrder) override {
        owner.setOrder(newOrder);
      }
      void setValidC(size_t i, double value) override { owner.c[i] = value; }
      void setValidD(size_t i, double value) override { owner.d[i] = value; }

    private:

      IIRCoefficients &owner;
    };

  public:
    Setter setter() {
      return Setter(*this);
    }

    IIRCoefficients(size_t maximumOrder, size_t initialOrder) :
            c(new coeff[allocationSize(maximumOrder)]),
            maxOrder(maximumOrder),
            d(c + maximumOrder + 1),
            order(validOrder(initialOrder, maxOrder)) {}

    explicit IIRCoefficients(size_t order) : IIRCoefficients(order, order) {}

    explicit IIRCoefficients(const IIRCoefficients &source) : IIRCoefficients(
            source.getMaxOrder(), source.getOrder()) {
      operator=(source);
    }

    IIRCoefficients(IIRCoefficients &&source) noexcept : IIRCoefficients(
            source.getMaxOrder(), source.getOrder()) {
      operator=(source);
    }

//    template<class Coefficients>
//    explicit IIRCoefficients(const Coefficients &source) : IIRCoefficients(
//            IIRCoefficientAccess<coeff, Coefficients>::getOrder(source),
//            IIRCoefficientAccess<coeff, Coefficients>::getMaxOrder(source)) {
//      operator=(source);
//    }

    IIRCoefficients &operator = (const IIRCoefficients &source) {
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

    template<class Coefficients>
    IIRCoefficients &operator = (const Coefficients &source) {
      using Access = IIRCoefficientAccess<coeff, Coefficients>;
      size_t sourceOrder = Access::getOrder(source);
      if (sourceOrder != order) {
        if (!isValidOrder(sourceOrder, maxOrder)) {
          throw std::invalid_argument("IIRCoefficients::operator=(source): "
                                      "cannot adopt source order (invalid).");
        }
      }
      order = sourceOrder;
      for (size_t i = 0; i <= order; i++) {
        c[i] = Access::getC(i, source);
        d[i] = Access::getD(i, source);
      }
      return *this;
    }

    sdsp_nodiscard sdsp_force_inline coeff getC(size_t i) const { return c[i]; }
    sdsp_nodiscard sdsp_force_inline coeff getD(size_t i) const { return d[i]; }
    sdsp_nodiscard sdsp_force_inline size_t getOrder() const { return order; }
    sdsp_nodiscard sdsp_force_inline size_t getMaxOrder() const { return maxOrder; }

    ~IIRCoefficients() {
      delete[] c;
      c = d = nullptr;
      maxOrder = order = 0;
    }

  protected:
    void setOrder(size_t newOrder) {
      order = validOrder(newOrder, maxOrder);
    }
    
  private:
    static size_t allocationSize(size_t order) {
      return Size<coeff>::validSumGet(1,
              Size<coeff>::validProductGet(
                      order, 2,
                      ValidGet::RESULT), ValidGet::RESULT);
    }

    static size_t validOrder(size_t order, size_t maxOrder) {
      if (isValidOrder(order, maxOrder)) {
        return order;
      }
      throw std::invalid_argument("IIRFixedOrderCoefficients::validOrder(order, maxOrder):"
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

#endif //SIMPLE_DSP_IIR_H
