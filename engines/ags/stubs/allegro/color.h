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

#ifndef AGS_STUBS_ALLEGRO_COLOR_H
#define AGS_STUBS_ALLEGRO_COLOR_H

#include "common/scummsys.h"
#include "ags/stubs/allegro/alconfig.h"

namespace AGS3 {

#include "common/pack-start.h"  // START STRUCT PACKING

struct color {
	byte r, g, b;
} PACKED_STRUCT;

typedef color RGB;
typedef color PALETTE[256];

#include "common/pack-end.h"    // END STRUCT PACKING

//define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

extern int bestfit_color(const PALETTE pal, int r, int g, int b);

extern int _rgb_r_shift_15;
extern int _rgb_g_shift_15;
extern int _rgb_b_shift_15;
extern int _rgb_r_shift_16;
extern int _rgb_g_shift_16;
extern int _rgb_b_shift_16;
extern int _rgb_r_shift_24;
extern int _rgb_g_shift_24;
extern int _rgb_b_shift_24;
extern int _rgb_r_shift_32;
extern int _rgb_g_shift_32;
extern int _rgb_b_shift_32;
extern int _rgb_a_shift_32;

AL_FUNC(void, set_color, (int idx, AL_CONST RGB *p));
AL_FUNC(void, set_palette, (AL_CONST PALETTE p));
AL_FUNC(void, set_palette_range, (AL_CONST PALETTE p, int from, int to, int retracesync));

} // namespace AGS3

#endif
