//
// Created by michel on 28-11-19.
//

#include <simple-dsp/guards.h>
#include <iostream>

namespace simpledsp {
  thread_local int NestedMemoryFence::level_;
  std::atomic<bool> MemoryFence::variable_;
}
