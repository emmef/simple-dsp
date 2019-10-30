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
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <stdio.h>
#include <vector>

namespace simpledsp {

  /**
   * Describes a single operation in a butterfly filter execution pattern. For example, consider
   * a multi-crossover that uses a list of filters from the lowest to the highest crossover
   * frequency. A butterfly entry gives the input index via @var{filter()}, the index of the filter
   * to use in @var{input()}, the output for the lower frequencies in @var{lowpass1()} and the
   * output of the higher frequencies in @var{highpass2()}. The lower pass should be done first,
   * as it never coincides with the input, while the high pass might.
   */
  class CrossoverProcessingStep {
    size_t filter_ = 0;
    size_t input_ = 0;
    size_t lowpass_ = 0;
    size_t highpass_ = 0;

  public:
    CrossoverProcessingStep() {}
    CrossoverProcessingStep(size_t input,
            size_t filter,
            size_t lowpass,
            size_t highpass) :
            filter_(filter), input_(input), lowpass_(lowpass), highpass_(highpass) {
    }

    /**
     * @return the operator index to be applied to and the outputs.
     */
    size_t filter() const
    {
      return filter_;
    }

    /**
     * @return the input index
     */
    size_t input() const {
      return input_;
    }

    /**
     * @return the output for the applied operator that contains the lower half as related to
     * the order of the operators.
     */
    size_t lowpass1() const {
      return lowpass_;
    }

    /**
     * @return the output for the applied operator that contains the higher half as related to
     * the order of the operators.
     */
    size_t highpass2() const {
      return highpass_;
    }
  };

  class CrossoverPlanCreator {

    template<typename T>
    static void addToPlan(
            T &entries,
            size_t size, size_t &index, size_t input, size_t output1, size_t output2) {
      size_t operators = size;
      entries[index++] = CrossoverProcessingStep(
              operators - input,
              operators - 1 - input,
              operators - output1,
              operators - output2);
    }

    template<typename T>
    static void createSubPlan(T &entries, size_t size, size_t &index, ssize_t min, ssize_t max) {
      if (min == max) {
        addToPlan(entries, size, index, min, min + 1, min);
        return;
      }
      ssize_t input = (min + max) / 2;
      ssize_t lo = std::max(input - 1, (ssize_t) min);
      ssize_t hi = std::min(input + 1, (ssize_t) max);
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
        throw std::invalid_argument("Need at least one entries in a butterfly plan");
      }
    }

  public:
    static void createPlan(CrossoverProcessingStep *entries, size_t size) {
      createPlanGeneric<CrossoverProcessingStep *>(entries, size);
    }

    static void createPlan(std::vector<CrossoverProcessingStep> &entries, size_t size) {
      checkValidSize(size);
      entries.resize(size);
      createPlanGeneric<std::vector<CrossoverProcessingStep>>(entries, size);
    }

    template<typename T>
    static void createPlanGeneric(T &entries, size_t size) {
      checkValidSize(size);
      size_t index = 0;
      createSubPlan(entries, size, index, 0, size - 1);
    }
    
    /**
     * Also create a "double" plan for, say, Linkwitz-Riley filters that works with buffers that
     * need to retain history for all values. This can be done by splitting entries. 
     * Imagine we have N frequency bands and thus N buffers. 
     * The following processing step: 
     *     to 4 apply filter 3 send lowpass to 2 and highpass to 6
     *     to 6 apply filter 5 send lowpass to 5 and highpass to 6
     *  can be split in 
     *     to 4 apply filter 3 send lowpass to N+1 and highpass to N+2
     *     to N+1 apply filter 3 send lowpass to 2
     *     to N+2 apply filter 3 send highpas to 6
     *     to 6 apply filter 5 send lowpass to N+3 and highpass to N+4
     *     to N+3 apply filter 5 send lowpass to 5
     *     to N+4 apply filter 5 send lowpass to 5
     *  This is a naive implementation. It is also possible to retain the history for the next 
     *  frame of buffers for each step, whereby we can suffice with having N+1 buffers for
     *  samples and 3N buffers (of length = order + 1) for history. The temporary buffer is
     *  called X.
     *  
     * The following processing step: 
     *     to [input] apply [filter] lowpass to [lowpass1] and highpass to [highpass2]
     *  can be split in:
     *
     *       prepend history[N+1] to [input]
     *       prepend history[N+2] to [lowpass1]
     *       prepend history[N+3] to [X]
     *       to [input] apply [filter]:lowpass to [X]
     *       to [X] apply [filter]:lowpass to [lowpass1]
     *       capture last samples of [X] to history[N+3]
     *       capture last samples of [lowpass1] to history[N+2]
     *       prepend history[N+4] to [highpass2]
     *       prepend history[N+5] to [X]
     *       to [input] apply [filter]:highpass to [X]
     *       to [X] apply [filter]:highpass to [highpass2]
     *       capture last samples of [X] to history[N+4]
     *       capture last samples of [highpass2] to history[N+5]
     *       capture last samples of [input] to history[N+1]
     *     
     *  If the values of input and highpass2 are equal, we cannot reorder these. If they are NOT,
     *  the process can be somewhat simplified to, though it is debatable whether it's worth the
     *  fuss:
     *
     *       prepend history[N+1] to [input]
     *       capture last samples of [input] to history[N+1]
     *       prepend history[N+4] to [highpass2]
     *       prepend history[N+2] to [lowpass1]
     *       prepend history[N+3] to [X]
     *       to [input] apply [filter]:lowpass to [X]
     *       to [X] apply [filter]:lowpass to [lowpass1]
     *       capture last samples of [lowpass1] to history[N+2]
     *       capture last samples of [X] to history[N+3]
     *       prepend history[N+5] to [X]
     *       to [input] apply [filter]:highpass to [X]
     *       to [X] apply [filter]:highpass to [highpass2]
     *       capture last samples of [X] to history[N+4]
     *       capture last samples of [highpass2] to history[N+5]
     *
     *  It is possible to remove some operations when combining multiple crossover steps. That's
     *  another exercise.
     */
  };

} // namespace simpledsp

#endif //SIMPLE_DSP_CROSSOVERPLAN_H
