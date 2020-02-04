#ifndef SIMPLE_DSP_ALIGNEDDATA_H
#define SIMPLE_DSP_ALIGNEDDATA_H
/*
 * simple-dsp/aligneddata.h
 *
 * Added by michel on 2019-09-12
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
#include <simple-dsp/addressing.h>
#include <simple-dsp/alignment.h>

namespace simpledsp {

template <typename T, size_t ALIGNMENT, class Data> struct AlignedData {
  using Metric = AlignedMetric<T, ALIGNMENT>;
  using ArrayIndex = Index::Array;
  using MethodIndex = Index::Method;

  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  sdsp_nodiscard sdsp_force_inline size_type size() const {
    return reinterpret_cast<const Data *>(this)->trait_size();
  }

  sdsp_nodiscard sdsp_force_inline const_pointer ptr() const {
    return Metric::assumeAligned(
        reinterpret_cast<const Data *>(this)->trait_unsafe_ptr());
  }

  sdsp_nodiscard sdsp_force_inline pointer ptr() {
    return Metric::assumeAligned(
        reinterpret_cast<Data *>(this)->trait_unsafe_ptr());
  }

  sdsp_nodiscard sdsp_force_inline pointer frame(size_type i) {
    return Metric::assumeAligned(ptr() +
                                 MethodIndex::index(i * ALIGNMENT, size()));
  }

  sdsp_nodiscard sdsp_force_inline const_pointer frame(size_type i) const {
    return Metric::assumeAligned(ptr() +
                                 MethodIndex::index(i * ALIGNMENT, size()));
  }

  sdsp_nodiscard sdsp_force_inline pointer operator()(size_type i) {
    return Metric::assumeAligned(ptr() +
                                 ArrayIndex::index(i * ALIGNMENT, size()));
  }

  sdsp_nodiscard sdsp_force_inline const_pointer operator()(size_type i) const {
    return Metric::assumeAligned(ptr() +
                                 ArrayIndex::index(i * ALIGNMENT, size()));
  }

  sdsp_nodiscard sdsp_force_inline const_reference
  operator[](size_type i) const {
    return ptr()[ArrayIndex::index(i, size())];
  }

  sdsp_nodiscard sdsp_force_inline reference operator[](size_type i) {
    return ptr()[ArrayIndex::index(i, size())];
  }

  sdsp_nodiscard sdsp_force_inline const_reference at(size_type i) const {
    return ptr()[MethodIndex::index(i, size())];
  }

  sdsp_nodiscard sdsp_force_inline reference at(size_type i) {
    return ptr()[MethodIndex::index(i, size())];
  }
};

namespace base {

template <typename T, size_t ALIGNMENT, class Data>
struct BaseAlignedData
    : public AlignedData<T, ALIGNMENT, BaseAlignedData<T, ALIGNMENT, Data>> {
  using Base = AlignedData<T, ALIGNMENT, BaseAlignedData<T, ALIGNMENT, Data>>;
  using Metric = typename Base::Metric;
  using ArrayIndex = typename Base::ArrayIndex;
  using MethodIndex = typename Base::MethodIndex;
  using size_type = typename Base::size_type;
  using pointer = typename Base::pointer;
  using const_pointer = typename Base::const_pointer;

  sdsp_nodiscard sdsp_force_inline constexpr size_type trait_size() const {
    return data_.trait_size();
  }

  sdsp_nodiscard sdsp_force_inline constexpr const pointer
  trait_unsafe_ptr() const {
    return Metric::assumeAligned(data_.trait_unsafe_ptr());
  }

  sdsp_nodiscard sdsp_force_inline constexpr const_pointer trait_unsafe_ptr() {
    return Metric::assumeAligned(data_.trait_unsafe_ptr());
  }

protected:
  Data data_;
};

template <typename T, size_t SIZE, size_t ALIGNMENT>
struct alignas(ALIGNMENT) BaseAlignedArrayData {
  alignas(ALIGNMENT) T data_[SIZE];
  using AlignedMetric = AlignedMetric<T, ALIGNMENT>;

  sdsp_nodiscard sdsp_force_inline T *trait_unsafe_ptr() {
    return AlignedMetric::assumeAligned(&data_);
  }

  sdsp_nodiscard sdsp_force_inline const T *trait_unsafe_ptr() const {
    return AlignedMetric::assumeAligned(&data_);
  }

  sdsp_nodiscard sdsp_force_inline size_t trait_size() const { return SIZE; }
};

template <typename T, size_t ALIGNMENT>
struct alignas(ALIGNMENT) BaseAlignedAllocatedData {
  using AlignedMetric = AlignedMetric<T, ALIGNMENT>;

  BaseAlignedAllocatedData(size_t size)
      : data_(new T(sizeof(T) * size,
                    static_cast<std::align_val_t>(ALIGNMENT))) {}

  sdsp_nodiscard sdsp_force_inline T *trait_unsafe_ptr() {
    return AlignedMetric::assumeAligned(data_);
  }

  sdsp_nodiscard sdsp_force_inline const T *trait_unsafe_ptr() const {
    return AlignedMetric::assumeAligned(data_);
  }

  sdsp_nodiscard sdsp_force_inline size_t trait_size() const { return size_; }

  ~BaseAlignedAllocatedData() { delete[] data_; }

private:
  T *data_;
  size_t size_;
};

} // namespace base

template <typename T, size_t SIZE, size_t ALIGNMENT>
using AlignedArray =
    base::BaseAlignedData<T, ALIGNMENT,
                          base::BaseAlignedArrayData<T, SIZE, ALIGNMENT>>;

template <typename T, size_t ALIGNMENT>
using AlignedBuffer =
    base::BaseAlignedData<T, ALIGNMENT,
                          base::BaseAlignedAllocatedData<T, ALIGNMENT>>;

template <typename T, size_t ALIGNMENT>
struct alignas(ALIGNMENT) AlignedPointer {
  using AlignedMetric = AlignedMetric<T, ALIGNMENT>;

  sdsp_nodiscard sdsp_force_inline T *ptr() {
    return AlignedMetric::assumeAligned(data_);
  }

  sdsp_nodiscard sdsp_force_inline const T *ptr() const {
    return AlignedMetric::assumeAligned(data_);
  }

  sdsp_nodiscard sdsp_force_inline operator T *() {
    return AlignedMetric::assumeAligned(data_);
  }

  sdsp_nodiscard sdsp_force_inline operator const T *() const {
    return AlignedMetric::assumeAligned(data_);
  }

  void setData(T *ptr) { data_ = AlignedMetric::verifiedAligned(ptr); }

  void operator=(T *ptr) { setData(ptr); }

private:
  T *data_ = nullptr;
};

} // namespace simpledsp

#endif // SIMPLE_DSP_ALIGNEDDATA_H
