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

/*
 *   sdsp_nodiscard static bool is_valid_delay

  sdsp_nodiscard static WrappedIndex get_index_for(size_type delay) {
    size_type effectiveDelay = Val::max(delay, min_delay());
    WrappedIndex::is_valid_element_count(effectiveDelay)
  }

 */

/**
 * Define how a delay is used, which also affects the possible delays, given a
 * buffer size.
 */
enum class DelayAccessType { READ_THEN_WRITE, WRITE_THEN_READ };

template <typename size_type, DelayAccessType access_type> struct DelayStatics;

template <typename size_type>
struct DelayStatics<size_type, DelayAccessType::READ_THEN_WRITE> {
  static constexpr size_type minimum = 1;

  static constexpr size_type elements_for_delay(size_type delay) noexcept {
    return delay;
  }

  static constexpr size_type elements_for_delay(size_type delay) noexcept {
    return delay;
  }


};

  template <DelayAccessType accessType, WrappingType wrappingType,
            size_t element_size, typename size_type, int max_size_bits>
  struct DelayBasics;

  template <WrappingType wrappingType, size_t element_size, typename size_type,
            int max_size_bits>
  struct DelayBasics<DelayAccessType::READ_THEN_WRITE, wrappingType,
                     element_size, size_type, max_size_bits> {

    using WrappedIndex =
        WrappedIndex<wrappingType, element_size, size_type, max_size_bits>;

    WrappedIndex index;

    static constexpr size_type min_delay_value = 1;
    static constexpr size_type max_delay_value =
        WrappedIndex::max_element_count;
    sdsp_nodiscard static bool is_valid_delay_value(size_type delay) {
      return Val::is_within(delay, min_delay_value, max_delay_value);
    }

    sdsp_nodiscard static constexpr size_type
    get_allocation_size(size_type delaySamples) {
      return WrappedIndex::get_allocation_size_for(delaySamples);
    }
  };

  DelayBasics(const WrappedIndex &wrappedIndex) : index(wrappedIndex) {}

  sdsp_nodiscard constexpr size_type getMaximumDelay() { return index.size(); }

  sdsp_nodiscard constexpr bool isValidDelay(size_type delaySamples) {
    return WrappedIndex::is_valid_element_count(delaySamples);
  }

  sdsp_nodiscard constexpr size_type getValidDelay(size_type delaySamples) {
    if (isValidDelay(delaySamples)) {
      return delaySamples;
    }
    throw std::invalid_argument("Basics::getDelta: delaySamples must lie "
                                "between 1 and the buffer size");
  }

  sdsp_nodiscard constexpr size_type getUncheckedDelta(size_type delaySamples) {
    return index.wrap(delaySamples);
  }

  sdsp_nodiscard size_type getDelta(size_type delaySamples) {
    return getUncheckedDelta(getValidDelay(delaySamples));
  }

  sdsp_nodiscard size_type getReadPtrForDelay(size_type writePtr,
                                              size_type delay) {
    return index.sub(writePtr, getValidDelay(delay));
  }

  sdsp_nodiscard size_type getWritePtrForDelay(size_type readPtr,
                                               size_type delay) {
    return index.add(readPtr, getValidDelay(delay));
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
}; // namespace simpledsp::util

template <WrappingType wrappingType, size_t element_size, typename size_type,
          int max_size_bits>
struct DelayBasics<DelayAccessType::WRITE_THEN_READ, wrappingType, element_size,
                   size_type, max_size_bits> {

  using WrappedIndex =
      WrappedIndex<wrappingType, element_size, size_type, max_size_bits>;
  WrappedIndex index;

  struct Static {
    sdsp_nodiscard static constexpr size_type min_delay() { return 0; }
    sdsp_nodiscard static constexpr size_type max_delay() {
      return WrappedIndex::max_element_count - 1;
    }
    sdsp_nodiscard static constexpr size_type
    getAllocationSize(size_type delaySamples) {
      return WrappedIndex::get_allocation_size_for(delaySamples + 1);
    }
  };

  DelayBasics(const WrappedIndex &wrappedIndex) : index(wrappedIndex) {}

  sdsp_nodiscard constexpr size_type getMaximumDelay() {
    return index.size() - 1;
  }

  sdsp_nodiscard constexpr bool isValidDelay(size_type delaySamples) {
    return Val::is_within(delaySamples, getMinimumDelay(), getMaximumDelay());
  }

  sdsp_nodiscard constexpr size_type getValidDelay(size_type delaySamples) {
    if (isValidDelay(delaySamples)) {
      return delaySamples;
    }
    throw std::invalid_argument("Basics::getDelta: delaySamples must lie "
                                "between 0 and the buffer size - 1");
  }

  sdsp_nodiscard static constexpr size_type
  getUncheckedDelta(size_type delaySamples) {
    return metric.wrap(delaySamples);
  }

  sdsp_nodiscard static size_type getReadPtrForDelay(size_type writePtr,
                                                     size_type delay) {
    size_type validDelay = getValidDelay(metric, delay);
    return metric.subtract(writePtr, validDelay);
  }

  sdsp_nodiscard static size_type getWritePtrForDelay(size_type readPtr,
                                                      size_type delay) {
    size_type validDelay = getValidDelay(metric, delay);
    return metric.add(readPtr, validDelay);
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

template <typename size_type, DelayAccessType accessType, class Container>
class DelayOffsetsBase {

public:
  using Basics = DelayBasics<accessType>;
  using Metric = WrappedIndex;

  template <typename... args>
  explicit DelayOffsetsBase(size_type initialMetricSize, args... arguments)
      : metric_(initialMetricSize), container_(arguments...) {}

  explicit DelayOffsetsBase(const DelayOffsetsBase &source) = delete;
  explicit DelayOffsetsBase(DelayOffsetsBase &&source) = delete;

  sdsp_nodiscard const Metric &getMetric() const { return metric_; }

  void next() {
    for (size_type &offset : container_) {
      metric_.setNext(offset);
    }
  }

  void reset() {
    for (size_type &offset : container_) {
      offset = 0;
    }
  }

  sdsp_nodiscard size_type size() const { return container_.size(); }

  sdsp_nodiscard size_type getOffset(size_type i) const {
    return container_.at(i);
  }

  sdsp_nodiscard size_type operator[](size_type i) const {
    return container_[i];
  }

  sdsp_nodiscard const size_type *ref(size_type i) const {
    return container_.data() + Index::safe(i, container_.size());
  }

  sdsp_nodiscard const size_type *operator+(size_type i) const {
    return container_.data() + Index::unsafe(i, container_.size());
  }

  sdsp_nodiscard size_type setWriteForDelay(size_type readIndex,
                                            size_type writeIndex,
                                            size_type delay) {
    return setAndGetWriteDelay(readIndex, writeIndex, delay);
  }

  sdsp_nodiscard size_type setReadForDelay(size_type readIndex,
                                           size_type writeIndex,
                                           size_type delay) {
    return setAndGetReadDelay(readIndex, writeIndex, delay);
  }

protected:
  sdsp_nodiscard Metric &metric() { return metric_; }

  Container &container() { return container_; }

  const Container &getContainer() const { return container_; }

private:
  Metric metric_;
  Container container_;

  size_type setAndGetWriteDelay(size_type readIndex, size_type writeIndex,
                                size_type delay) {
    size_type result = metric_.add(container_.at(readIndex),
                                   Basics::getValidDelay(metric_, delay));
    container_.at(writeIndex) = metric_.add(
        container_.at(readIndex), Basics::getValidDelay(metric_, delay));
    return result;
  }

  size_type setAndGetReadDelay(size_type readIndex, size_type writeIndex,
                               size_type delay, bool check) {
    size_type result = check ? Basics::getValidDelay(metric_, delay)
                             : Basics::getUncheckedDelta(metric_, delay);
    container_.at(readIndex) =
        metric_.subtract(container_.at(writeIndex), result);
    return result;
  }
};

} // namespace simpledsp::util

#endif // SIMPLE_DSP_UTIL_DELAY_H
