#ifndef SIMPLE_DSP_LOCK_FEE_OWNER_HPP
#define SIMPLE_DSP_LOCK_FEE_OWNER_HPP
/*
 * simple-dsp/lock-free-owner.hpp
 *
 * Added by michel on 2019-11-27
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

#include <chrono>
#include <cstddef>
#include <random>
#include <simple-dsp/guards.h>
#include <simple-dsp/queue.h>
#include <simple-dsp/timeout.h>
#include <thread>

namespace simpledsp {
enum class LockFeeOwnerResult { SUCCESS, INVALID, TIMEOUT };

/**
 * Managers ownership of an Object that can be used by a thread that needs to
 * work in a lock-free contex, e.g. a real-time thread.
 *
 * Fetching of the object in #get() is always lock-free.
 *
 * Setting or constructng a new "version" of the object must be done in a thread
 * where memory allocation, a busy lock and yielding thread control is
 * permitted.
 * @tparam Object The type of object to manage
 */
template <typename Object> class LockfreeOwner {
  GuardedFlag flag_;
  std::atomic<Object *> current_ = nullptr;
  std::atomic<Object *> next_ = nullptr;
  QueueProducerConsumer<Object *> queue_;

public:
  LockfreeOwner(size_t destructionQueueSize) : queue_(destructionQueueSize) {}

  LockfreeOwner() : LockfreeOwner(10) {}

  /**
   * Gets the current object. This is a lock-free operation even if a new
   * "version" has been set and can be used from a real-time context.
   *
   * @return The current object.
   */
  sdsp_nodiscard Object *get() noexcept {
    auto guard = flag_.guard();
    if (!guard.isSet()) {
      return current_;
    }
    Object *current = current_;
    Object *next = next_;
    if (!next_.compare_exchange_strong(next, nullptr)) {
      return current;
    }
    if (!next) {
      return current;
    }
    if (current) {
      if (queue_.put(current_) != QueueResult::SUCCESS) {
        next_ = next;
        return current;
      }
    }
    // Make sure this thread "sees" all constructed data.
    MemoryFence::acquire();
    current_ = next;
    return next;
  }

  /**
   * Gets the current object without regard for newer verisons and with a full
   * memory fence. The latter might seem overkill, but the get() method only
   * ensures visibility and consistency for the thread that calls it, which
   * often is a real-time processing thread. This method allows other threads to
   * inspect the object in a non-modifiable fashion.
   *
   * @return The current object or nullptr if it was not set.
   */
  sdsp_nodiscard const Object *getCurrent() noexcept {
    MemoryFence::acquire();
    return current_;
  }

  /**
   * Sets a new object, that will be OWNED by this LockFreeAtomicOwner. If
   * setting the object does not work at once, there will be a
   * busy-yield-spinlock for at most the specified number of microseconds. If
   * setting the object fails, it will be deleted by this method.
   *
   * Because of the lock, this method MUST NOT be used by real-time threads.
   *
   * @param timeOut The time out, allowed to set the object.
   * @param object The new "version" of the object to set.
   * @return Result::SUCCESS on success and another Result otherwise
   */
  LockFeeOwnerResult set(TimeOut &timeOut, Object *object) {
    return setProduced(timeOut, passObject, object);
  }

  /**
   * Constructs a new object. If setting the object does not work at once, there
   * will be a busy-yield-spinlock for at most the specified number of
   * microseconds.
   *
   * @tparam ConstructorArguments The types of constructor parameters
   * @param timeOut The time out, allowed to set the object.
   * @param arguments Actual arguments, used to construct the object.
   * @return Result::SUCCESS on success and another Result otherwise
   */
  template <typename... ConstructorArguments>
  LockFeeOwnerResult construct(TimeOut &timeOut,
                               ConstructorArguments... arguments) {
    return setProduced<ConstructorArguments...>(
        timeOut, constructObject<ConstructorArguments...>, arguments...);
  }

  /**
   * Cleans up previous "versions" of the managed object.
   */
  void cleanup() {
    while (true) {
      auto guard = flag_.guard();
      if (guard.isSet()) {
        Object *object = nullptr;
        if (queue_.get(object) == QueueResult::SUCCESS) {
          if (object) {
            MemoryFence::acquire();
            delete object;
          }
        } else {
          return;
        }
      }
    }
  }

  ~LockfreeOwner() {
    MemoryFence fence;
    cleanup();
    deleteOnceNotNull(current_);
    deleteOnceNotNull(next_);
  }

private:
  template <typename... ProducerParameters>
  LockFeeOwnerResult
  setProduced(TimeOut &timeOut,
              Object *(*objectProducer)(ProducerParameters...),
              ProducerParameters... parameters) {
    if (!objectProducer) {
      return LockFeeOwnerResult::INVALID;
    }
    Object *object = nullptr;
    timeOut.start();
    do {
      if (!object) {
        object = objectProducer(parameters...);
        if (!object) {
          return LockFeeOwnerResult::INVALID;
        }
      }
      {
        auto guard = flag_.guard();
        if (guard.isSet()) {
          Object *expected = nullptr;
          if (next_.compare_exchange_strong(expected, object)) {
            // ensure that produced object is visible to all threads that use
            // acquire fence.
            MemoryFence fence;
            cleanupUnsafe();
            return LockFeeOwnerResult::SUCCESS;
          }
        }
      }
    } while (timeOut.inTime());
    delete object;
    return LockFeeOwnerResult::TIMEOUT;
  }

  static Object *passObject(Object *object) { return object; }

  template <typename... ConstructorArguments>
  static Object *constructObject(ConstructorArguments... arguments) {
    return new Object(arguments...);
  }

  void cleanupUnsafe() {
    while (true) {
      Object *object = nullptr;
      // make sure destructor "sees" all data that might have
      // See comment about atomic: this might be moved inside the block where
      // queue fetch was successful.
      if (queue_.get(object) == QueueResult::SUCCESS) {
        if (object) {
          MemoryFence::acquire();
          delete object;
        }
      } else {
        return;
      }
    }
  }
};

} // namespace simpledsp

#endif // SIMPLE_DSP_LOCK_FEE_OWNER_HPP
