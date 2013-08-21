/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011, 2012, 2013 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MT32EMU_MT32EMU_H
#define MT32EMU_MT32EMU_H

// Debugging

// 0: Standard debug output is not stamped with the rendered sample count
// 1: Standard debug output is stamped with the rendered sample count
// NOTE: The "samplestamp" corresponds to the end of the last completed rendering run.
//       This is important to bear in mind for debug output that occurs during a run.
#define MT32EMU_DEBUG_SAMPLESTAMPS 0

// 0: No debug output for initialisation progress
// 1: Debug output for initialisation progress
#define MT32EMU_MONITOR_INIT 0

// 0: No debug output for MIDI events
// 1: Debug output for weird MIDI events
#define MT32EMU_MONITOR_MIDI 0

// 0: No debug output for note on/off
// 1: Basic debug output for note on/off
// 2: Comprehensive debug output for note on/off
#define MT32EMU_MONITOR_INSTRUMENTS 0

// 0: No debug output for partial allocations
// 1: Show partial stats when an allocation fails
// 2: Show partial stats with every new poly
// 3: Show individual partial allocations/deactivations
#define MT32EMU_MONITOR_PARTIALS 0

// 0: No debug output for sysex
// 1: Basic debug output for sysex
#define MT32EMU_MONITOR_SYSEX 0

// 0: No debug output for sysex writes to the timbre areas
// 1: Debug output with the name and location of newly-written timbres
// 2: Complete dump of timbre parameters for newly-written timbres
#define MT32EMU_MONITOR_TIMBRES 0

// 0: No TVA/TVF-related debug output.
// 1: Shows changes to TVA/TVF target, increment and phase.
#define MT32EMU_MONITOR_TVA 0
#define MT32EMU_MONITOR_TVF 0

// Configuration

// If non-zero, deletes reverb buffers that are not in use to save memory.
// If zero, keeps reverb buffers for all modes around all the time to avoid allocating/freeing in the critical path.
#define MT32EMU_REDUCE_REVERB_MEMORY 1

// 0: Maximum speed at the cost of a bit lower emulation accuracy.
// 1: Maximum achievable emulation accuracy.
#define MT32EMU_BOSS_REVERB_PRECISE_MODE 0

// 0: Use 16-bit signed samples and refined wave generator based on logarithmic fixed-point computations and LUTs. Maximum emulation accuracy and speed.
// 1: Use float samples in the wave generator and renderer. Maximum output quality and minimum noise.
#define MT32EMU_USE_FLOAT_SAMPLES 0

namespace MT32Emu
{
// The default value for the maximum number of partials playing simultaneously.
const unsigned int DEFAULT_MAX_PARTIALS = 32;

// The higher this number, the more memory will be used, but the more samples can be processed in one run -
// various parts of sample generation can be processed more efficiently in a single run.
// A run's maximum length is that given to Synth::render(), so giving a value here higher than render() is ever
// called with will give no gain (but simply waste the memory).
// Note that this value does *not* in any way impose limitations on the length given to render(), and has no effect
// on the generated audio.
// This value must be >= 1.
const unsigned int MAX_SAMPLES_PER_RUN = 4096;

// The default size of the internal MIDI event queue.
// It holds the incoming MIDI events before the rendering engine actually processes them.
// The main goal is to fairly emulate the real hardware behaviour which obviously
// uses an internal MIDI event queue to gather incoming data as well as the delays
// introduced by transferring data via the MIDI interface.
// This also facilitates building of an external rendering loop
// as the queue stores timestamped MIDI events.
const unsigned int DEFAULT_MIDI_EVENT_QUEUE_SIZE = 1024;
}

#include "Structures.h"
#include "common/file.h"
#include "Tables.h"
#include "Poly.h"
#include "LA32Ramp.h"
#include "LA32WaveGenerator.h"
#include "TVA.h"
#include "TVP.h"
#include "TVF.h"
#include "Partial.h"
#include "Part.h"
#include "ROMInfo.h"
#include "Synth.h"

#endif
