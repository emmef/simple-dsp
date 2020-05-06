#ifndef SIMPLE_DSP_DSPNODE_H
#define SIMPLE_DSP_DSPNODE_H
/*
 * simple-dsp/dspnode.h
 *
 * Added by michel on 2020-02-09
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
namespace simpledsp {

enum class DspState {
  /**
   * Initial state after construction. Constructor MUST throw if this state
   * cannot be attained. This also assumes that interface-related
   * initialisation has been done.
   */
  INIT,
  /**
   * State after configuration with user configuration. How this information
   * is conveyed to the actual DSP processing is left to the implementation.
   * The DSP is not running at this time.
   */
  CONFIGURED,
  /**
   * DSP is running. The DSP can be signalled to stop via the STOP status, in
   * which case the DSP will complete one cycle and then set the state to
   * CONFIGURED.
   */
  ACTIVE,
  /**
   * Signals the DSP to complete a cycle and then set the state to CONFIGURED.
   */
  STOP
};


} // namespace simpledsp

#endif // SIMPLE_DSP_DSPNODE_H
