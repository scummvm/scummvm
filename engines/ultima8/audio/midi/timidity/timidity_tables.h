/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef USE_TIMIDITY_MIDI

#ifndef ULTIMA8_AUDIO_MIDI_TIMIDITY_TIMIDITY_TABLES_H
#define ULTIMA8_AUDIO_MIDI_TIMIDITY_TIMIDITY_TABLES_H

#include "timidity.h"

#include <cmath>

namespace Ultima8 {

#ifdef NS_TIMIDITY
namespace NS_TIMIDITY {
#endif

#ifdef LOOKUP_SINE
extern float sine(int x);
#else
#define sine(x) (std::sin((2*PI/1024.0) * (x)))
#endif

#define SINE_CYCLE_LENGTH 1024
extern int32 freq_table[];
extern double vol_table[];
extern double bend_fine[];
extern double bend_coarse[];
extern uint8 *_l2u; /* 13-bit PCM to 8-bit u-law */
extern uint8 _l2u_[]; /* used in LOOKUP_HACK */
#ifdef LOOKUP_HACK
extern int16 _u2l[];
extern int32 *mixup;
#ifdef LOOKUP_INTERPOLATION
extern int8 *iplookup;
#endif
#endif

extern void init_tables(void);

#ifdef NS_TIMIDITY
};
#endif

} // End of namespace Ultima8

#endif

#endif
