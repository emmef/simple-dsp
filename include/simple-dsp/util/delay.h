#ifndef SIMPLE_DSP_UTIL_DELAY_H
#define SIMPLE_DSP_UTIL_DELAY_H
/*
 * simple-dsp/util/delay.h
 *
 * Added by michel on 2019-08-20
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

#include <array>
#include <simple-dsp/core/bounds.h>
#include <simple-dsp/core/circular.h>
#include <simple-dsp/core/index.h>
#include <vector>

namespace simpledsp::util {

/**
 * Define how a delay is used, which also affects the possible delays, given a
 * buffer size.
 */
enum class DelayAccessType { READ_THEN_WRITE, WRITE_THEN_READ };

template <DelayAccessType accessType> struct DelayBasics;

template <> struct DelayBasics<DelayAccessType::READ_THEN_WRITE> {

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType getMinimumDelay() {
    return 1;
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType
  getMaximumDelay(const CircularMetric<SizeType> &metric) {
    return metric.getSize();
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr bool
  isValidDelay(const CircularMetric<SizeType> &metric, SizeType delaySamples) {
    return is_within(delaySamples, getMinimumDelay<SizeType>(),
                     getMaximumDelay<SizeType>(metric));
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType
  getUncheckedDelta(const CircularMetric<SizeType> &metric,
                    SizeType delaySamples) {
    return metric.wrap(delaySamples);
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType
  getValidDelay(const CircularMetric<SizeType> &metric, SizeType delaySamples) {
    if (isValidDelay<SizeType>(metric, delaySamples)) {
      return delaySamples;
    }
    throw std::invalid_argument("Basics::getDelta: delaySamples must lie "
                                "between 1 and the buffer size");
  }

  template <typename SizeType>
  sdsp_nodiscard static SizeType
  getDelta(const CircularMetric<SizeType> &metric, SizeType delaySamples) {
    return getUncheckedDelta<SizeType>(
        metric, getValidDelay<SizeType>(metric, delaySamples));
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType
  getAllocationSize(SizeType delaySamples) {
    return CircularArithmic<SizeType>::proper_circular_size(delaySamples);
  }

  template <typename SizeType>
  sdsp_nodiscard static SizeType
  getReadPtrForDelay(const CircularMetric<SizeType> &metric, SizeType writePtr,
                     SizeType delay) {
    SizeType validDelay = getValidDelay(metric, delay);
    return metric.subtract(writePtr, validDelay);
  }

  template <typename SizeType>
  sdsp_nodiscard static SizeType
  getWritePtrForDelay(const CircularMetric<SizeType> &metric, SizeType readPtr,
                      SizeType delay) {
    SizeType validDelay = getValidDelay(metric, delay);
    return metric.add(readPtr, validDelay);
  }

  template <typename SizeType>
  sdsp_nodiscard static CircularMetric<SizeType>
  createMetricFor(SizeType delaySamples) {
    return CircularArithmic<SizeType>(delaySamples);
  }

  template <typename Sample>
  sdsp_nodiscard static Sample access(Sample *write, const Sample *read,
                                      Sample value) {
    Sample result = *read;
    *write = value;
    return result;
  }

  template <typename Sample>
  sdsp_nodiscard static Sample access(Sample &write, const Sample &read,
                                      Sample value) {
    Sample result = read;
    write = value;
    return result;
  }
};

template <> struct DelayBasics<DelayAccessType::WRITE_THEN_READ> {

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType getMinimumDelay() {
    return 0;
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType
  getMaximumDelay(const CircularMetric<SizeType> &metric) {
    return metric.getSize() - 1;
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType
  getAllocationSize(SizeType delaySamples) {
    return CircularArithmic<SizeType>::proper_circular_size(delaySamples + 1);
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr bool
  isValidDelay(const CircularMetric<SizeType> &metric, SizeType delaySamples) {
    return is_within(delaySamples, getMinimumDelay<SizeType>(),
                     getMaximumDelay<SizeType>(metric));
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType
  getUncheckedDelta(const CircularMetric<SizeType> &metric,
                    SizeType delaySamples) {
    return metric.wrap(delaySamples);
  }

  template <typename SizeType>
  sdsp_nodiscard static constexpr SizeType
  getValidDelay(const CircularMetric<SizeType> &metric, SizeType delaySamples) {
    if (isValidDelay<SizeType>(metric, delaySamples)) {
      return delaySamples;
    }
    throw std::invalid_argument("Basics::getDelta: delaySamples must lie "
                                "between 0 and the buffer size - 1");
  }

  template <typename SizeType>
  sdsp_nodiscard static SizeType
  getReadPtrForDelay(const CircularMetric<SizeType> &metric, SizeType writePtr,
                     SizeType delay) {
    SizeType validDelay = getValidDelay(metric, delay);
    return metric.subtract(writePtr, validDelay);
  }

  template <typename SizeType>
  sdsp_nodiscard static SizeType
  getWritePtrForDelay(const CircularMetric<SizeType> &metric, SizeType readPtr,
                      SizeType delay) {
    SizeType validDelay = getValidDelay(metric, delay);
    return metric.add(readPtr, validDelay);
  }

  template <typename SizeType>
  sdsp_nodiscard static CircularMetric<SizeType>
  createMetricFor(SizeType delaySamples) {
    return CircularArithmic<SizeType>(delaySamples + 1);
  }

  template <typename Sample>
  sdsp_nodiscard static Sample access(Sample *write, const Sample *read,
                                      Sample value) {
    *write = value;
    return *read;
  }

  template <typename Sample>
  sdsp_nodiscard static Sample access(Sample &write, const Sample &read,
                                      Sample value) {
    write = value;
    return read;
  }
};

template <typename SizeType, DelayAccessType accessType, class Container>
class DelayOffsetsBase {

public:
  using Basics = DelayBasics<accessType>;
  using Metric = CircularMetric<SizeType>;

  template <typename... args>
  explicit DelayOffsetsBase(SizeType initialMetricSize, args... arguments)
      : metric_(initialMetricSize), container_(arguments...) {}

  explicit DelayOffsetsBase(const DelayOffsetsBase &source) = delete;
  explicit DelayOffsetsBase(DelayOffsetsBase &&source) = delete;

  sdsp_nodiscard const Metric &getMetric() const { return metric_; }

  void next() {
    for (SizeType &offset : container_) {
      metric_.setNext(offset);
    }
  }

  void reset() {
    for (SizeType &offset : container_) {
      offset = 0;
    }
  }

  sdsp_nodiscard SizeType size() const { return container_.size(); }

  sdsp_nodiscard SizeType getOffset(SizeType i) const {
    return container_.at(i);
  }

  sdsp_nodiscard SizeType operator[](SizeType i) const { return container_[i]; }

  sdsp_nodiscard const SizeType *ref(SizeType i) const {
    return container_.data() + Index::safe(i, container_.size());
  }

  sdsp_nodiscard const SizeType *operator+(SizeType i) const {
    return container_.data() + Index::unsafe(i, container_.size());
  }

  sdsp_nodiscard SizeType setWriteForDelay(SizeType readIndex,
                                           SizeType writeIndex,
                                           SizeType delay) {
    return setAndGetWriteDelay(readIndex, writeIndex, delay);
  }

  sdsp_nodiscard SizeType setReadForDelay(SizeType readIndex,
                                          SizeType writeIndex, SizeType delay) {
    return setAndGetReadDelay(readIndex, writeIndex, delay);
  }

protected:
  sdsp_nodiscard Metric &metric() { return metric_; }

  Container &container() { return container_; }

  const Container &getContainer() const { return container_; }

private:
  Metric metric_;
  Container container_;

  SizeType setAndGetWriteDelay(SizeType readIndex, SizeType writeIndex,
                               SizeType delay) {
    SizeType result = metric_.add(container_.at(readIndex),
                                  Basics::getValidDelay(metric_, delay));
    container_.at(writeIndex) = metric_.add(
        container_.at(readIndex), Basics::getValidDelay(metric_, delay));
    return result;
  }

  SizeType setAndGetReadDelay(SizeType readIndex, SizeType writeIndex,
                              SizeType delay, bool check) {
    SizeType result = check ? Basics::getValidDelay(metric_, delay)
                            : Basics::getUncheckedDelta(metric_, delay);
    container_.at(readIndex) =
        metric_.subtract(container_.at(writeIndex), result);
    return result;
  }
};

} // namespace simpledsp::util

#endif // SIMPLE_DSP_UTIL_DELAY_H
