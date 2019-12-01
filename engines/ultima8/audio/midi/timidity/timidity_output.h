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

#ifndef ULTIMA8_AUDIO_MIDI_TIMIDITY_TIMIDITY_OUTPUT_H
#define ULTIMA8_AUDIO_MIDI_TIMIDITY_TIMIDITY_OUTPUT_H

#include "timidity.h"

namespace Ultima8 {

#ifdef NS_TIMIDITY
namespace NS_TIMIDITY {
#endif

struct PlayMode {
	int32 rate, encoding;
	const char *id_name;
};

extern PlayMode *play_mode_list[], *play_mode;
extern int init_buffers(int kbytes);

/* Conversion functions -- These overwrite the int32 data in *lp with
   data in another format */

/* The size of the output buffers */
extern int AUDIO_BUFFER_SIZE;

/* Actual copy function */
extern void (*s32tobuf)(void *dp, int32 *lp, int32 c);

/* 8-bit signed and unsigned*/
extern void s32tos8(void *dp, int32 *lp, int32 c);
extern void s32tou8(void *dp, int32 *lp, int32 c);

/* 16-bit */
extern void s32tos16(void *dp, int32 *lp, int32 c);
extern void s32tou16(void *dp, int32 *lp, int32 c);

/* byte-exchanged 16-bit */
extern void s32tos16x(void *dp, int32 *lp, int32 c);
extern void s32tou16x(void *dp, int32 *lp, int32 c);

/* uLaw (8 bits) */
extern void s32toulaw(void *dp, int32 *lp, int32 c);

/* little-endian and big-endian specific */
#ifdef TIMIDITY_LITTLE_ENDIAN
#define s32tou16l s32tou16
#define s32tou16b s32tou16x
#define s32tos16l s32tos16
#define s32tos16b s32tos16x
#else
#define s32tou16l s32tou16x
#define s32tou16b s32tou16
#define s32tos16l s32tos16x
#define s32tos16b s32tos16
#endif

#ifdef NS_TIMIDITY
};
#endif

} // End of namespace Ultima8

#endif

#endif
