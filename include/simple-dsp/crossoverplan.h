#ifndef SIMPLE_DSP_CROSSOVERPLAN_H
#define SIMPLE_DSP_CROSSOVERPLAN_H
/*
 * simple-dsp/crossoverplan.h
 *
 * Added by michel on 2019-10-22
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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <simple-dsp/addressing.h>
#include <stdexcept>
#include <stdio.h>
#include <vector>

namespace simpledsp {

/**
 * Describes a crossover implementation plan, using the "butterfly" pattern. A
 * crossover that output N frequency bands has N - 1 crossover filters. The plan
 * assumes that both the output frequency ands and the filters are sorted from
 * the lowest to the highest frequency. Let M be N if N is a power of 2 the next
 * power of 2 that is greater than N otherwise. The butterfly plan ensures that
 * each frequency band passes thourgh <em>at most</em> 2log(M) filters. This is
 * obtained in N - 1 steps, that are described by a #Plan object.
 */
class CrossoverPlan {
  template <typename T>
  static void addToPlan(T &entries, size_t size, size_t &index, size_t input,
                        size_t output1, size_t output2) {
    size_t operators = size;
    entries[index++] = Step(operators - input, operators - 1 - input,
                            operators - output1, operators - output2);
  }

  template <typename T>
  static void createSubPlan(T &entries, size_t size, size_t &index, ssize_t min,
                            ssize_t max) {
    if (min == max) {
      addToPlan(entries, size, index, min, min + 1, min);
      return;
    }
    ssize_t input = (min + max) / 2;
    ssize_t lo = std::max(input - 1, (ssize_t)min);
    ssize_t hi = std::min(input + 1, (ssize_t)max);
    ssize_t loOut = (min + lo) / 2;
    ssize_t hiOut = (max + hi) / 2;

    addToPlan(entries, size, index, input, hiOut, loOut);

    if (lo >= min && lo != input) {
      createSubPlan(entries, size, index, min, lo);
    }
    if (hi <= max && hi != input) {
      createSubPlan(entries, size, index, hi, max);
    }
  }

  static void checkValidSize(size_t size) {
    if (size < 1) {
      throw std::invalid_argument(
          "Need at least one entries in a butterfly plan");
    }
  }

public:
  /**
   * Defines one step in a butterfly crossover plan. In pseudo code, processing
   * can be done as follows: \code input = sample[step.input()] filter =
   * filters[step.filter()] sample[step.lowOut()] = filter.lowpass(input)
   * sample[step.highOut()] = filter.highpass(input)
   * \endcode
   * It is important to realise that step.highpass2() and step.input() can be
   * the same index: therefore low-pass and high-pass should never be swapped!
   * If your filter is phase-free and the high-pass part can be obtain by
   * subtracting it from the original, the step can be simplified as follows:
   * \code
   * input = sample[step.input()]
   * filter = filters[step.filter()]
   * sample[step.lowOut()] = filter.lowpass(input)
   * sample[step.highOut()] = input - sample[step.lowOut()]
   * \endcode
   */
  class Step {
    size_t filter_ = 0;
    size_t input_ = 0;
    size_t lowOut_ = 0;
    size_t highOut_ = 0;

  public:
    Step() {}
    Step(size_t input, size_t filter, size_t lowOut, size_t highOut)
        : filter_(filter), input_(input), lowOut_(lowOut), highOut_(highOut) {}

    /**
     * @return the filter-index to be applied to the input with index #input()
     * to obtain outputs.
     */
    sdsp_nodiscard size_t filter() const { return filter_; }

    /**
     * @return the input-index.
     */
    sdsp_nodiscard size_t input() const { return input_; }

    /**
     * Returns the low-pass output index. Filtered information should always be
     * written to this output before the output is written to the output with
     * index #highOut(), as that might overwrite the input.
     * @return the index of the output when the low-pass part of the filter with
     * index #filter() is applied to the input with index #input().
     */
    sdsp_nodiscard size_t lowOut() const { return lowOut_; }

    /**
     * Returns the high-pass output index. Filtered information should always be
     * written to the output with index #lowOut() before it is written to this
     * one, as this one might overwrite the input.
     * @return the index of the output when the high-pass part of the filter
     * with index #filter() is applied to the input with index #input().
     */
    sdsp_nodiscard size_t highOut() const { return highOut_; }
  };

  /**
   * Creates a crossover plan with \a crossover crossovers.
   * @param crossovers The number of crossovers, that must be one or larger
   */
  CrossoverPlan(size_t crossovers)
      : steps_(new Step[Size<Step>::validGet(crossovers)]),
        crossovers_(crossovers) {
    create(steps_, crossovers);
  }

  const Step &operator[](size_t idx) const {
    return steps_[Index::Array::index(idx, crossovers_)];
  }

  const Step &at(size_t idx) const {
    return steps_[Index::Method::index(idx, crossovers_)];
  }

  const Step *begin() const noexcept { return steps_; }

  const Step *cbegin() const noexcept { return steps_; }

  ~CrossoverPlan() {
    if (steps_) {
      delete[] steps_;
      crossovers_ = 0;
    }
  }

  /**
   * Creates a butterfly plan for a crossover with \a crossover crossovers and
   * thus \a crossover + 1 frequency bands.
   * @param steps A buffer that can contain at least \a crossover steps.
   * @param crossovers The number of crossovers that must be larger than zero
   * and yields (crossover + 1) frequency bands.
   */
  static void create(Step *steps, size_t crossovers) {
    createGeneric<Step *>(steps, crossovers);
  }

  /**
   * Creates a butterfly plan for a crossover with \a crossover crossovers and
   * thus \a crossover + 1 frequency bands.
   * @param steps A vector that will have size \a crossovers and contains the
   * #{Step}s.
   * @param crossovers The number of crossovers that must be larger than zero
   * and yields (crossover + 1) frequency bands.
   */
  static void create(std::vector<Step> &steps, size_t crossovers) {
    checkValidSize(crossovers);
    steps.resize(crossovers);
    steps.begin();
    createGeneric<std::vector<Step>>(steps, crossovers);
  }

  /**
   * Creates a butterfly plan for a crossover with \a crossover crossovers and
   * thus \a crossover + 1 frequency bands.
   * @param steps An object that has an operator \code{.cpp}
   * Step &operator[](size_t)
   * \endcode.
   * @param crossovers The number of crossovers that must be larger than zero
   * and yields (crossover + 1) frequency bands.
   */
  template <typename T> static void createGeneric(T &steps, size_t crossovers) {
    checkValidSize(crossovers);
    size_t index = 0;
    createSubPlan(steps, crossovers, index, 0, crossovers - 1);
  }

private:
  Step *steps_;
  size_t crossovers_;

  /**
   * Also create a "double" plan for, say, Linkwitz-Riley filters that works
   * with buffers that need to retain history for the next "frame" of buffers.
   * This can be done by splitting entries.
   *
   * Imagine we have N frequency bands and thus N buffers.
   * The following processing step number M (0-based)
   *
   *     to [input] apply [filter] lowpass to [lowpass1] and highpass to
   * [highpass2]
   *
   *  can be split in
   *
   *       prepend history[5M + 1] to [input]
   *       prepend history[5M + 2] to [lowpass1]
   *       prepend history[5M + 3] to [X]
   *       to [input] apply [filter]:lowpass to [X]
   *       to [X] apply [filter]:lowpass to [lowpass1]
   *       capture last samples of [X] to history[5M + 3]
   *       capture last samples of [lowpass1] to history[5M + 2]
   *       prepend history[5M + 4] to [highpass2]
   *       prepend history[5M + 5] to [X]
   *       to [input] apply [filter]:highpass to [X]
   *       to [X] apply [filter]:highpass to [highpass2]
   *       capture last samples of [X] to history[5M + 4]
   *       capture last samples of [highpass2] to history[5M + 5]
   *       capture last samples of [input] to history[5M + 1]
   *
   *  It is possible to remove some capture-prepend pairs from the complete
   * plan, which also reduces the number of history buffers. But that is for
   * advanced users.
   */
};

} // namespace simpledsp

#endif // SIMPLE_DSP_CROSSOVERPLAN_H
