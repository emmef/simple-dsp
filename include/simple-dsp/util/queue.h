#ifndef SIMPLE_DSP_UTIL_QUEUE_H
#define SIMPLE_DSP_UTIL_QUEUE_H
/*
 * simple-dsp/util/queue.h
 *
 * Added by michel on 2019-11-21
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

#include <limits>
#include <simple-dsp/core/index.h>
#include <simple-dsp/util/guards.h>
#include <stdexcept>
#include <type_traits>

namespace simpledsp::util {

namespace queue_position {

/**
 * Queue positions manage the read and write position of a queue. All positions
 * must implement the methods in this Traits class.
 *
 * The implementations simpledsp::queue_position::Unsafe,
 * simpledsp::queue_position::Atomic and simpledsp::queue_position::Guarded are
 * all lock-free.
 *
 * @TODO: Actually use this class to test if the position implementations have
 * these methods.
 */
template <class Implementation> struct PositionTraits {
  /**
   * Enters the context where positions are used and reports if that was
   * successful. If it was successful, it also loads the read and write
   * positions.
   *
   * If this method returns false, the caller is free to move on and it MUST NOT
   * call leave(). If the method returns true, the caller MUST invoke exactly
   * one of #storeRead, #storeWrite or #leave().
   *
   * @param rd Contains the read position on success.
   * @param wr Contains the write position on success.
   * @return true if loading the positions was successful.
   */
  sdsp_nodiscard sdsp_force_inline bool enterAndLoad(size_t &rd,
                                                     size_t &wr) noexcept {
    return reinterpret_cast<Implementation *>(this)->enterAndLoadTrait(rd, wr);
  }

  /**
   * Stores the read position.
   *
   * The caller MUST have invoked #enterAndLoad(size_t&,size_t&) successfully
   * before using this method.
   *
   * @param newReadPosition The value to store in the read pointer.
   */
  sdsp_force_inline void storeRead(size_t newReadPosition) noexcept {
    return reinterpret_cast<Implementation *>(this)->storeReadTrait(
        newReadPosition);
  }

  /**
   * Stores the write position.
   *
   * The caller MUST have invoked #enterAndLoad(size_t&,size_t&) successfully
   * before using this method.
   *
   * @param newWritePosition The value to store in the write pointer.
   */
  sdsp_force_inline void storeWrite(size_t newWritePosition) noexcept {
    return reinterpret_cast<Implementation *>(this)->storeWriteTrait(
        newWritePosition);
  }

  /**
   * Leaves the context in which manipulation to positions (and data) happens.
   *
   * If this method is not invoked, there is no guarantee that other threads
   * will see any changes in the queue position or content. In case of a guarded
   * position, not invoking this method can leave the queue inaccessible
   * forever.
   */
  sdsp_force_inline void leave() noexcept {
    return reinterpret_cast<Implementation *>(this)->leaveTrait();
  }
};

/**
 * A position implementation that offers no guarantees at all concerning memory
 * visibility or queue position consistency.
 *
 * Queues that use this implementation are usefull when wrapped in a queue that
 * does full synchronisation with mutexes.
 */
class Unsafe : public PositionTraits<Unsafe> {
  size_t wr_ = 0;
  size_t rd_ = 0;

public:
  sdsp_force_inline bool enterAndLoadTrait(size_t &rd, size_t &wr) noexcept {
    rd = rd_;
    wr = wr_;
    return true;
  }

  sdsp_force_inline void storeReadTrait(size_t value) noexcept { rd_ = value; }

  sdsp_force_inline void storeWriteTrait(size_t value) noexcept { wr_ = value; }

  sdsp_force_inline void leaveTrait() noexcept {}
};

/**
 * A position implementation that offers memory visibility but not necessarily
 * consistency. This implementation can be used for queues where all puts happen
 * in one thread and all gets happen in one thread. If that cannot be
 * guaranteed, use the Guarded variant.
 */
class Atomic : public PositionTraits<Atomic> {
  std::atomic<size_t> wr_ = 0;
  std::atomic<size_t> rd_ = 0;

public:
  sdsp_force_inline bool enterAndLoadTrait(size_t &rd, size_t &wr) noexcept {
    rd = rd_.load(std::memory_order_relaxed);
    wr = wr_.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    return true;
  }

  sdsp_force_inline void storeReadTrait(size_t value) noexcept {
    std::atomic_thread_fence(std::memory_order_release);
    rd_.store(value, std::memory_order_relaxed);
  }

  sdsp_force_inline void storeWriteTrait(size_t value) noexcept {
    std::atomic_thread_fence(std::memory_order_release);
    wr_.store(value, std::memory_order_relaxed);
  }

  sdsp_force_inline void leaveTrait() noexcept { util::MemoryFence::release(); }
};

/**
 * A position implementation that offers memory visibility and consistency.
 */
class Consistent : public PositionTraits<Consistent> {
  std::atomic<size_t> wr_ = 0;
  std::atomic<size_t> rd_ = 0;
  std::atomic_flag busy_ = ATOMIC_FLAG_INIT;

public:
  sdsp_force_inline bool enterAndLoadTrait(size_t &rd, size_t &wr) noexcept {
    if (busy_.test_and_set()) {
      return false;
    }
    rd = rd_.load(std::memory_order_relaxed);
    wr = wr_.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    return true;
  }

  sdsp_force_inline void storeReadTrait(size_t value) noexcept {
    std::atomic_thread_fence(std::memory_order_release);
    rd_.store(value, std::memory_order_relaxed);
    busy_.clear();
  }

  sdsp_force_inline void storeWriteTrait(size_t value) noexcept {
    std::atomic_thread_fence(std::memory_order_release);
    wr_.store(value, std::memory_order_relaxed);
    busy_.clear();
  }

  sdsp_force_inline void leaveTrait() noexcept {
    std::atomic_thread_fence(std::memory_order_release);
    busy_.clear();
  }
};
} // namespace queue_position

namespace queue_data {
/**
 * Contains the values for a queue. The number of values uf the capacity() and
 * the maximum value that can be passed to the #operator[](size_t) operators is
 * that capacity. This means that for a straightforward implementation, the
 * allocated size should be the capacity plus one.
 * @tparam Value The type of value
 * @todo Verify traits for actual data storages
 */
template <typename Value, typename Implementation> class DataTraits {
public:
  /**
   * Returns the capacity of the queue when using this data. This should be one
   * less than the allocated size.
   * @return the capacity of the queue.
   */
  sdsp_nodiscard size_t capacity() const noexcept {
    return reinterpret_cast<const Implementation *>(this)->capacityTrait();
  };

  /**
   * Returns a non-constant reference to the object at the provided offset.
   * @param offset The offset of the value to reference.
   * @return a non-constant reference to the object at the provided offset.
   */
  sdsp_nodiscard Value &operator[](size_t offset) noexcept {
    return reinterpret_cast<Implementation *>(this)->refTrait(offset);
  };

  /**
   * Returns a constant reference to the object at the provided offset.
   * @param offset The offset of the value to reference.
   * @return a constant reference to the object at the provided offset.
   */
  sdsp_nodiscard const Value &operator[](size_t offset) const noexcept {
    return reinterpret_cast<const Implementation *>(this)->refTrait(offset);
  };
};
template <typename Value>
class DefaultData : public DataTraits<Value, DefaultData<Value>> {

  static size_t validCapacity(size_t capacity) {
    if (capacity > 0 && capacity <= maxCapacity) {
      return capacity;
    }
    throw std::invalid_argument(
        "BaseQueue<V>: Given capacity zero or too large for value type.");
  }

  SizeFor<Value> capacity_;
  Value *data_;

public:
  DefaultData(size_t capacity)
      : capacity_(validCapacity(capacity)), data_(new Value[capacity_ + 1lu]) {}
  static constexpr size_t maxCapacity = SizeFor<Value>::max_index;

  sdsp_nodiscard size_t capacityTrait() const noexcept { return capacity_; }

  sdsp_nodiscard Value &refTrait(size_t index) noexcept {
    return data_[Index::unsafe(index, capacity_())];
  }

  sdsp_nodiscard const Value &refTrait(size_t index) const noexcept {
    return data_[Index::unsafe(index, capacity_())];
  }

  ~DefaultData() { delete[] data_; }
};
} // namespace queue_data

enum class QueueResult { SUCCESS = 0, BUSY, FULL, EMPTY };

/**
 * A queue implementation, based on the provided Position and Allocator.
 *
 * @tparam Value The type of values to store, that must be trivial to construct,
 * copy and assign.
 * @tparam Position The type that managers the read and write positions and
 * provides specified visibility and consistency guarantees.
 * @tparam Data The type of storage to use, see
 * simpledsk::queue_data::DataTraits
 * @see simpledsp::queue_position::PositionTraits
 * @see simpledsp::queue_data::DataTraits
 */
template <typename Value, typename Position, typename Data> class BaseQueue {
  static_assert(std::is_trivially_copy_assignable<Value>::value,
                "Value type must be trivially copy-assignable");

  static_assert(
      std::is_base_of<queue_position::PositionTraits<Position>,
                      Position>::value,
      "Position is no subclass of queue_position::PositionTraits<Position>.");
  static_assert(
      std::is_base_of<queue_data::DataTraits<Value, Data>, Data>::value,
      "Data is no subclass of queue_data::DataTraits<Value,Data>.");

  using PositionTraits = queue_position::PositionTraits<Position>;

public:
  /**
   * Creates a queue whose storage is determined by the arguments given to the
   * underlying storage type.
   * @see the data
   * @param capacity The maximum number of elements in the queue.
   */
  template <typename... DataParameters>
  BaseQueue(DataParameters... parameters) : data_(parameters...) {}

  /**
   * Returns the capacity of the queue: the maximum number of elements it can
   * hold.
   * @return The capacity of the queue.
   */
  sdsp_nodiscard size_t capacity() const noexcept { return data_.capacity(); }

  /**
   * Returns the queue size: the current number of elements in the queue.
   *
   * If the position type used does not guarantee consistence and no other
   * measures are taken by the caller, the returned value can be wrong.
   *
   * @return The queue size.
   */
  sdsp_nodiscard ssize_t size() noexcept {
    size_t rd;
    size_t wr;
    if (position.enterAndLoad(rd, wr)) {
      if (wr < rd) {
        wr += data_.capacity() + 1;
      }
      position.leave();
      return wr - rd;
    }
    return -1;
  }

  /**
   * Returns the queue state, that indicates if putting or getting elements will
   * work.
   *
   * The results can be QueueResult#BUSY (come back later), QueueResult#FULL or
   * QueueResult#EMPTY.
   *
   * If the position type used does not guarantee consistence and no other
   * measures are taken by the caller, the returned value can be wrong.
   *
   * @return The queue state.
   */
  sdsp_nodiscard QueueResult state() noexcept {
    size_t rd;
    size_t wr;
    size_t next;
    if (!position.enterAndLoad(rd, wr)) {
      return QueueResult::BUSY;
    }

    if (isFull(rd, wr, next)) {
      position.leave();
      return QueueResult::FULL;
    }
    if (isEmpty(rd, wr)) {
      position.leave();
      return QueueResult::EMPTY;
    }
    position.leave();
    return QueueResult::SUCCESS;
  }

  /**
   * Puts a value and returns QueueResult#SUCCESS or returns an error result.
   * When the Queue uses an Atomic or Consistent position, all values put will
   * be guaranteed to be visible by any get that gets them.
   * @param value The value to put
   * @return QueueResult#SUCCESS, QueueResult#BUSY or QueueResult#FULL.
   */
  QueueResult put(const Value &value) noexcept {
    size_t rd;
    size_t wr;
    size_t next;
    if (!position.enterAndLoad(rd, wr)) {
      return QueueResult::BUSY;
    }
    if (isFull(rd, wr, next)) {
      position.leave();
      return QueueResult::FULL;
    }
    data_[wr] = value;
    position.storeWrite(next);
    return QueueResult::SUCCESS;
  }

  /**
   * Gets a value and returns QueueResult#SUCCESS or returns an error result.
   * When the Queue uses an Atomic or Consistent position, all got values will
   * be guaranteed to be completely visible with what was put previously.
   * @param value Contains the value on success.
   * @return QueueResult#SUCCESS, QueueResult#BUSY or QueueResult#FULL.
   */
  QueueResult get(Value &value) noexcept {
    size_t rd;
    size_t wr;
    if (!position.enterAndLoad(rd, wr)) {
      return QueueResult::BUSY;
    }

    if (isEmpty(rd, wr)) {
      position.leave();
      return QueueResult::EMPTY;
    }
    value = data_[rd];
    position.storeRead(nextValue(rd));
    return QueueResult::SUCCESS;
  }

  /**
   * Empties the queue and return whether that was successful.
   *
   * This method can be dangerous to use with a position implementation that
   * does not guarantee position consistency and where the caller does not
   * provide such a guarantee.
   * @return QueueResult#BUSY or QueueResult#SUCCESS.
   */
  QueueResult clear() noexcept {
    size_t rd;
    size_t wr;
    if (!position.enterAndLoad(rd, wr)) {
      return QueueResult::BUSY;
    }
    position.storeRead(0);
    position.storeWrite(0);
    position.leave();
    return QueueResult::SUCCESS;
  }

private:
  Position position;
  Data data_;

  sdsp_nodiscard size_t nextValue(size_t ptr) const noexcept {
    return ptr < data_.capacity() ? ptr + 1 : 0;
  }

  sdsp_nodiscard bool isFull(size_t rd, size_t wr,
                             size_t &next) const noexcept {
    next = nextValue(wr);
    return next == rd;
  }

  sdsp_nodiscard bool isEmpty(size_t rd, size_t wr) const noexcept {
    return wr == rd;
  }
};

/**
 * A queue that does not guarantee visibility nor protects against data races.
 * This type of queue should only be used in a single thread at both ends.
 * @see BaseQueue
 */
template <typename Value>
using QueueUnsafe =
    BaseQueue<Value, queue_position::Unsafe, queue_data::DefaultData<Value>>;

/**
 * A queue that ensures visibility but does not protect against put-put or
 * get-get data races. This type of queue should have one producer and one
 * consumer thread.
 * @see BaseQueue
 */
template <typename Value>
using QueueProducerConsumer =
    BaseQueue<Value, queue_position::Atomic, queue_data::DefaultData<Value>>;

template <typename Value>
using Queue = BaseQueue<Value, queue_position::Consistent,
                        queue_data::DefaultData<Value>>;

} // namespace simpledsp::util

#endif // SIMPLE_DSP_UTIL_QUEUE_H
