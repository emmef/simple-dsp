#ifndef SIMPLE_DSP_QUEUE_H
#define SIMPLE_DSP_QUEUE_H
/*
 * simple-dsp/queue.h
 *
 * Added by michel on 2019-11-21
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

#include <limits>
#include <stdexcept>
#include <type_traits>
#include <simple-dsp/spinguard.h>

namespace simpledsp {
  /**
   * A lock-free queue implementation. <em>This queue assumes that there is only one thread that
   * puts values and one thread that gets values.</em> If this assumption is not met, it is better
   * to use a Queue.
   *
   * @tparam T The type of values to store, that must be trivial to construct, copy and assign.
   */
  template<typename T>
  class QueueSingleThreaded {
    static_assert(
            std::is_trivially_copy_assignable<T>::value,
            "Value type must be trivially copy-assignable");
    static_assert(
            std::is_trivially_constructible<T>::value,
            "Value type must be trivial to construct");
    static_assert(
            std::atomic<int>::is_always_lock_free,
            "Position type atomic is not lock_free");

    T *data_;
    int size_; // data size is one larger, so size also functions as end value
    std::atomic<int> rd_ = 0;
    std::atomic<int> wr_ = 0;

    static int validSize(int size) {
      if (size > 0 && size < std::numeric_limits<int>::max() / size) {
        return size;
      }
      throw std::invalid_argument("Queue: Invalid size.");
    }

    [[nodiscard]] inline int nextValue(int ptr) const noexcept {
      return ptr < size_ ? ptr + 1 : 0; }

    [[nodiscard]] inline bool isFull(int &rd, int &wr, int &next) const noexcept {
      rd = rd_;
      wr = wr_;
      next = nextValue(wr);

      return next == rd;
    }

    [[nodiscard]] inline bool isEmpty(int &rd, int &wr) const noexcept {
      rd = rd_;
      wr = wr_;

      return wr == rd;
    }

  public:
    using type = T;

    explicit QueueSingleThreaded(int size) :
            data_(new T[validSize(size) + 1]),
            size_(size) {

    }

    ~QueueSingleThreaded() {
      delete [] data_;
    }

    /**
     * @return the maximum number of elements in the queue.
     */
    [[nodiscard]] int maxElements() const noexcept {
      return size_;
    }

    /**
     * Returns the current number of elements in the queue, that can be inaccurate because of
     * concurrent reads or writes.
     *
     * @return the current number of elements in the queue.
     */
    [[nodiscard]] int size() const noexcept {
      int rd = rd_;
      int wr = wr_;
      if (wr < rd) {
        wr += size_ + 1;
      }
      return wr - rd;
    }

    /**
     * Returns whether it is certain that a value can be put. Because this method does not block, it
     * can return false even if another thread just read a value and the queue if not full anymore.
     * @return true if it is certainly possible to put a value.
     */
    [[nodiscard]] bool canPut() const noexcept {
      int rd;
      int wr;
      int next;

      return !isFull(rd, wr, next);
    }

    /**
     * Returns whether it is certain that a value can be obtained. Because this method does not block,
     * it can return false even if another thread just wrote a value and the queue is not empty
     * anymore.
     * @return true if it is certainly possible to put a value.
     */
    [[nodiscard]] bool canGet() const noexcept {
      int rd;
      int wr;
      return !isEmpty(rd, wr);
    }

    /**
     * Puts a value if possible.
     * @param value The value to put
     * @return true if the value was put.
     * @see canPut()
     */
    bool put(const T &value) noexcept {
      int rd;
      int wr;
      int next;

      if (isFull(rd, wr, next)) {
        return false;
      }
      data_[wr] = value;
      wr_ = next;
      return true;
    }

    /**
     * Gets a value if possible.
     * @param value The value to get
     * @return true if the value was obtained.
     * @see canGet()
     */
    bool get(T &value) noexcept {
      int rd;
      int wr;
      if (isEmpty(rd, wr)) {
        return false;
      }
      int next = nextValue(rd);
      value = data_[rd];
      rd_ = next;
      return true;
    }
  };

  /**
   * A lock-free queue implementation. If there is only one
   * thread that puts values and one thread that gets values, it is better to use a
   * QueueSingleThreaded.
   *
   * @tparam T The type of values to store, that must be trivial to construct, copy and assign.
   */
  template <typename T>
  class Queue {
    QueueSingleThreaded<T> queue_;
    simpledsp::GuardedFlag flag_;
  public:
    Queue(int size) : queue_(size) {

    }
    /**
     * @return the maximum number of elements in the queue.
     */
    [[nodiscard]] int maxElements() const noexcept {
      return queue_.size();
    }

    /**
     * Returns the current number of elements in the queue, that can be inaccurate because of
     * concurrent reads or writes.
     *
     * @return the current number of elements in the queue.
     */
    [[nodiscard]] int size() const noexcept {
      return queue_.size();
    }

    /**
     * Returns whether it is certain that a value can be put. Because this method does not block, it
     * can return false even if another thread just read a value and the queue if not full anymore.
     * @return true if it is certainly possible to put a value.
     */
    [[nodiscard]] bool canPut() const noexcept {
      return queue_.canPut();
    }

    /**
     * Returns whether it is certain that a value can be obtained. Because this method does not block,
     * it can return false even if another thread just wrote a value and the queue is not empty
     * anymore.
     * @return true if it is certainly possible to put a value.
     */
    [[nodiscard]] bool canGet() const noexcept {
      return queue_.canGet();
    }

    /**
     * Puts a value if possible.
     * @param value The value to put
     * @param spins The number of attempts to make sure this thread is the only thread that puts or
     * gets values.
     * @return true if the value was put.
     * @see canPut()
     */
    bool put(const T &value, int spins) noexcept {
      simpledsp::SpinGuard guard = flag_.guard(spins);
      if (guard.isSet()) {
        return queue_.put(value);
      }
      return false;
    }

    /**
     * Puts a value if possible.
     * @param value The value to put
     * @return true if the value was put.
     * @see canPut()
     */
    bool put(const T &value) noexcept {
      return put(value, 1);
    }

    /**
     * Gets a value if possible.
     * @param value The value to get
     * @param spins The number of attempts to make sure this thread is the only thread that puts or
     * gets values.
     * @return true if the value was obtained.
     * @see canGet()
     */
    bool get(T &value, int spins) noexcept {
      simpledsp::SpinGuard guard = flag_.guard(spins);
      if (guard.isSet()) {
        return queue_.get(value);
      }
      return false;
    }

    /**
     * Gets a value if possible.
     * @param value The value to get
     * @return true if the value was obtained.
     * @see canGet()
     */
    bool get(T &value) noexcept {
      return get(value, 1);
    }
  };

} // namespace simpledsp

#endif //SIMPLE_DSP_QUEUE_H
