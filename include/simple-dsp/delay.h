#ifndef SIMPLE_DSP_DELAY_H
#define SIMPLE_DSP_DELAY_H
/*
 * simple-dsp/delay.h
 *
 * Added by michel on 2019-08-20
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

#include <simple-dsp/algorithm/circular.h>
#include <simple-dsp/algorithm/ranges.h>
#include <array>
#include <vector>

namespace simpledsp {

    /**
     * Define how a delay is used, which also affects the possible delays, given a buffer size.
     */
    enum class DelayAccessType {
        READ_THEN_WRITE,
        WRITE_THEN_READ
    };

    template <DelayAccessType accessType>
    struct DelayBasics {
    };

    template <>
    struct DelayBasics<DelayAccessType::READ_THEN_WRITE> {

        template<typename SizeType>
        sdsp_nodiscard static constexpr SizeType getMinimumDelay() {
            return 1;
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr SizeType getMaximumDelay(
                const algorithm::CircularMetricBase<SizeType> &metric) {
            return metric.getSize();
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr bool isValidDelay(
                const algorithm::CircularMetricBase <SizeType> &metric, SizeType delaySamples) {
            return algorithm::is_within(delaySamples, getMinimumDelay<SizeType>(), getMaximumDelay<SizeType>(metric));
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr SizeType getUncheckedDelta(
                const algorithm::CircularMetricBase <SizeType> &metric, SizeType delaySamples) {
            return metric.wrap(delaySamples);
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr bool getValidDelay(
                const algorithm::CircularMetricBase <SizeType> &metric, SizeType delaySamples) {
            if (isValidDelay<SizeType>(metric, delaySamples)) {
                return delaySamples;
            }
            throw std::invalid_argument("Basics::getDelta: delaySamples must lie between 1 and the buffer size");
        }

        template<typename SizeType>
        sdsp_nodiscard static SizeType getDelta(
                const algorithm::CircularMetricBase<SizeType> &metric, SizeType delaySamples) {
                return getUncheckedDelta<SizeType>(metric, getValidDelay<SizeType>(metric, delaySamples));
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr SizeType getAllocationSize(SizeType delaySamples) {
            return algorithm::CircularArithmicBase<SizeType>::properCircularSize(delaySamples);
        }

        template<typename SizeType>
        sdsp_nodiscard static algorithm::CircularMetricBase<SizeType> createMetricFor(SizeType delaySamples) {
            return algorithm::CircularMetricBase<SizeType>(delaySamples);
        }

        template<typename Sample>
        sdsp_nodiscard static Sample access(Sample *write, const Sample *read, Sample value) {
            Sample result = *read;
            *write = value;
            return result;
        }

        template<typename Sample>
        sdsp_nodiscard static Sample access(Sample &write, const Sample &read, Sample value) {
            Sample result = read;
            write = value;
            return result;
        }
    };

    template <>
    struct DelayBasics<DelayAccessType::WRITE_THEN_READ> {

        template<typename SizeType>
        sdsp_nodiscard static constexpr SizeType getMinimumDelay() {
            return 0;
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr SizeType getMaximumDelay(
                const algorithm::CircularMetricBase<SizeType> &metric) {
            return metric.getSize() - 1;
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr SizeType getAllocationSize(SizeType delaySamples) {
            return algorithm::CircularArithmicBase<SizeType>::properCircularSize(delaySamples + 1);
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr bool isValidDelay(
                const algorithm::CircularMetricBase <SizeType> &metric, SizeType delaySamples) {
            return algorithm::is_within(delaySamples, getMinimumDelay<SizeType>(), getMaximumDelay<SizeType>(metric));
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr SizeType getUncheckedDelta(
                const algorithm::CircularMetricBase <SizeType> &metric, SizeType delaySamples) {
            return metric.wrap(delaySamples);
        }

        template<typename SizeType>
        sdsp_nodiscard static constexpr bool getValidDelay(
                const algorithm::CircularMetricBase <SizeType> &metric, SizeType delaySamples) {
            if (isValidDelay<SizeType>(metric, delaySamples)) {
                return delaySamples;
            }
            throw std::invalid_argument("Basics::getDelta: delaySamples must lie between 0 and the buffer size - 1");
        }

        template<typename SizeType>
        sdsp_nodiscard static SizeType getDelta(
                const algorithm::CircularMetricBase<SizeType> &metric, SizeType delaySamples) {
            return getUncheckedDelta<SizeType>(metric, getValidDelay<SizeType>(metric, delaySamples));
        }


        template<typename SizeType>
        sdsp_nodiscard static algorithm::CircularMetricBase<SizeType> createMetricFor(SizeType delaySamples) {
            return algorithm::CircularMetricBase<SizeType>(delaySamples + 1);
        }

        template<typename Sample>
        sdsp_nodiscard static Sample access(Sample *write, const Sample *read, Sample value) {
            *write = value;
            return *read;
        }

        template<typename Sample>
        sdsp_nodiscard static Sample access(Sample &write, const Sample &read, Sample value) {
            write = value;
            return read;
        }
    };

    template <typename SizeType, DelayAccessType accessType, class Container>
    class DelayOffsetsBase {


    public:
        using Basics = DelayBasics<accessType>;
        using Metric = algorithm::CircularMetricBase<SizeType>;

        template<typename... args>
        explicit DelayOffsetsBase(SizeType initialMetricSize, args... arguments) : metric_(initialMetricSize), container_(arguments...) {
        }

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

        sdsp_nodiscard SizeType operator[](SizeType i) const {
            return container_[i];
        }

        sdsp_nodiscard const SizeType *ref(SizeType i) const {
            return container_.data() + algorithm::Index::index(i, container_.size());
        }

        void setWriteForDelay(SizeType readIndex, SizeType writeIndex, SizeType delay) {
            setAndGetWriteDelay(readIndex, writeIndex, delay, true);
        }

        sdsp_nodiscard SizeType setAndGetWriteForDelay(SizeType readIndex, SizeType writeIndex, SizeType delay) {
            return setAndGetWriteDelay(readIndex, writeIndex, delay, false);
        }

        void setReadForDelay(SizeType readIndex, SizeType writeIndex, SizeType delay) {
            setAndGetReadDelay(readIndex, writeIndex, delay, true);
        }

        sdsp_nodiscard SizeType setAndGetReadForDelay(SizeType readIndex, SizeType writeIndex, SizeType delay) {
            return setAndGetReadDelay(readIndex, writeIndex, delay, false);
        }

    protected:
        sdsp_nodiscard Metric &metric()  {
            return metric_;
        }

        Container &container() {
            return container_;
        }

        const Container &getContainer() const {
            return container_;
        }

    private:
        Metric metric_;
        Container container_;

        SizeType setAndGetWriteDelay(SizeType readIndex, SizeType writeIndex, SizeType delay, bool check) {
            SizeType result = check
                              ? Basics::getValidDelay(metric_, delay)
                              : Basics::getUncheckedDelta(metric_, delay);
            container_.at(writeIndex) = metric_.add(container_.at(readIndex), result);
            return result;
        }

        SizeType setAndGetReadDelay(SizeType readIndex, SizeType writeIndex, SizeType delay, bool check) {
            SizeType result = check
                              ? Basics::getValidDelay(metric_, delay)
                              : Basics::getUncheckedDelta(metric_, delay);
            container_.at(readIndex) = metric_.subtract(container_.at(writeIndex), result);
            return result;
        }
    };

    template <typename SizeType, SizeType Size, DelayAccessType accessType>
    class ArrayDelayOffsetsContainerBase
            : public DelayOffsetsBase<SizeType, accessType, std::array<SizeType, Size>> {

        using Parent = DelayOffsetsBase<SizeType, accessType, std::array<SizeType, Size>>;

    protected:

        explicit ArrayDelayOffsetsContainerBase(SizeType initialMetricSize)
                :  Parent(initialMetricSize) {
            Parent::reset();
        }
    };

    template <typename SizeType, DelayAccessType accessType>
    class VectorDelayOffsetsContainerBase
            : public DelayOffsetsBase<SizeType, accessType, std::vector<SizeType>> {

        using Parent = DelayOffsetsBase<SizeType, accessType, std::vector<SizeType>>;

    protected:

        VectorDelayOffsetsContainerBase(SizeType initialMetricSize, SizeType numberOfOffsets)
                :  Parent(initialMetricSize, numberOfOffsets) {
            Parent::reset();
        }
    };

} // namespace simpledsp

#endif //SIMPLE_DSP_DELAY_H
