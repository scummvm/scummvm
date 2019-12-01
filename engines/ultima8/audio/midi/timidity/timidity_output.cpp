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

#include "ultima8/misc/pent_include.h"

#ifdef USE_TIMIDITY_MIDI

#include "timidity.h"
#include "timidity_output.h"
#include "timidity_tables.h"

namespace Ultima8 {

#ifdef NS_TIMIDITY
namespace NS_TIMIDITY {
#endif

#ifdef SDL
extern PlayMode sdl_play_mode;
#define DEFAULT_PLAY_MODE &sdl_play_mode
#endif

PlayMode *play_mode_list[] = {
#ifdef DEFAULT_PLAY_MODE
	DEFAULT_PLAY_MODE,
#endif
	0
};

#ifdef DEFAULT_PLAY_MODE
PlayMode *play_mode = DEFAULT_PLAY_MODE;
#endif

/*****************************************************************/
/* Some functions to convert signed 32-bit data to other formats */

void s32tos8(void *dp, int32 *lp, int32 c) {
	int8 *cp = (int8 *)(dp);
	int32 l;
	while (c--) {
		l = (*lp++) >> (32 - 8 - GUARD_BITS);
		if (l > 127) l = 127;
		else if (l < -128) l = -128;
		*cp++ = (int8)(l);
	}
}

void s32tou8(void *dp, int32 *lp, int32 c) {
	uint8 *cp = (uint8 *)(dp);
	int32 l;
	while (c--) {
		l = (*lp++) >> (32 - 8 - GUARD_BITS);
		if (l > 127) l = 127;
		else if (l < -128) l = -128;
		*cp++ = 0x80 ^ ((uint8) l);
	}
}

void s32tos16(void *dp, int32 *lp, int32 c) {
	int16 *sp = (int16 *)(dp);
	int32 l;
	while (c--) {
		l = (*lp++) >> (32 - 16 - GUARD_BITS);
		if (l > 32767) l = 32767;
		else if (l < -32768) l = -32768;
		*sp++ = (int16)(l);
	}
}

void s32tou16(void *dp, int32 *lp, int32 c) {
	uint16 *sp = (uint16 *)(dp);
	int32 l;
	while (c--) {
		l = (*lp++) >> (32 - 16 - GUARD_BITS);
		if (l > 32767) l = 32767;
		else if (l < -32768) l = -32768;
		*sp++ = 0x8000 ^ (uint16)(l);
	}
}

void s32tos16x(void *dp, int32 *lp, int32 c) {
	int16 *sp = (int16 *)(dp);
	int32 l;
	while (c--) {
		l = (*lp++) >> (32 - 16 - GUARD_BITS);
		if (l > 32767) l = 32767;
		else if (l < -32768) l = -32768;
		*sp++ = XCHG_SHORT((int16)(l));
	}
}

void s32tou16x(void *dp, int32 *lp, int32 c) {
	uint16 *sp = (uint16 *)(dp);
	int32 l;
	while (c--) {
		l = (*lp++) >> (32 - 16 - GUARD_BITS);
		if (l > 32767) l = 32767;
		else if (l < -32768) l = -32768;
		*sp++ = XCHG_SHORT(0x8000 ^ (uint16)(l));
	}
}

void s32toulaw(void *dp, int32 *lp, int32 c) {
	uint8 *up = (uint8 *)(dp);
	int32 l;
	while (c--) {
		l = (*lp++) >> (32 - 13 - GUARD_BITS);
		if (l > 4095) l = 4095;
		else if (l < -4096) l = -4096;
		*up++ = _l2u[l];
	}
}

#ifdef NS_TIMIDITY
};
#endif

} // End of namespace Ultima8

#endif //USE_TIMIDITY_MIDI
