/* Copyright (c) 2003-2005 Various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MT32EMU_MT32EMU_H
#define MT32EMU_MT32EMU_H

// Debugging
// Show the instruments played
#define MT32EMU_MONITOR_INSTRUMENTS 1
// Shows number of partials MT-32 is playing, and on which parts
#define MT32EMU_MONITOR_PARTIALS 0
// Determines how the waveform cache file is handled (must be regenerated after sampling rate change)
#define MT32EMU_WAVECACHEMODE 0 // Load existing cache if possible, otherwise generate and save cache
//#define MT32EMU_WAVECACHEMODE 1 // Load existing cache if possible, otherwise generage but don't save cache
//#define MT32EMU_WAVECACHEMODE 2 // Ignore existing cache, generate and save cache
//#define MT32EMU_WAVECACHEMODE 3 // Ignore existing cache, generate but don't save cache

// Configuration
// The maximum number of partials playing simultaneously
#define MT32EMU_MAX_PARTIALS 32
// The maximum number of notes playing simultaneously per part.
// No point making it more than MT32EMU_MAX_PARTIALS, since each note needs at least one partial.
#define MT32EMU_MAX_POLY 32
// This calculates the exact frequencies of notes as they are played, instead of offsetting from pre-cached semitones. Potentially very slow.
#define MT32EMU_ACCURATENOTES 0

#if (defined (_MSC_VER) && defined(_M_IX86))
#define MT32EMU_HAVE_X86
#elif  defined(__GNUC__)
#if __GNUC__ >= 3 && defined(__i386__)
#define MT32EMU_HAVE_X86
#endif
#endif

#ifdef MT32EMU_HAVE_X86
#define MT32EMU_USE_MMX 1
#else
#define MT32EMU_USE_MMX 0
#endif

#include "freeverb.h"

#include "structures.h"
#include "i386.h"
#include "mt32_file.h"
#include "tables.h"
#include "partial.h"
#include "partialManager.h"
#include "part.h"
#include "synth.h"

#endif
