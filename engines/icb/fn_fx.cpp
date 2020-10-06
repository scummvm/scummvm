/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"

namespace ICB {

#define DEFAULT_WIDESCREEN_SPEED_ON 6
#define DEFAULT_WIDESCREEN_MODE_ON 0
#define DEFAULT_WIDESCREEN_SPEED_OFF 6
#define DEFAULT_WIDESCREEN_MODE_OFF 0

mcodeFunctionReturnCodes fx_widescreen(int32 &, int32 *params) {
	// fx_narrow_screen(mode,0,0,0,target,6
	// where mode is 0 for on and 2 for off
	// where target is 27% for on and 0% for off
	int32 p[6] = {DEFAULT_WIDESCREEN_MODE_OFF, 0, 0, 0, 0, DEFAULT_WIDESCREEN_SPEED_OFF};

	// if on alter arguments 0 (mode) and 4 (percent)
	if (params[0]) {
		p[0] = DEFAULT_WIDESCREEN_MODE_ON;  // scroll on
		p[4] = 27;                          // 27% is 32 pixels off top and bottom
		p[5] = DEFAULT_WIDESCREEN_SPEED_ON; // speed
	}

	int32 ret;
	return MS->fx_narrow_screen(ret, p);
}

mcodeFunctionReturnCodes _simple_fx(int32 mode, int32 toFrom, int32 fr, int32 fg, int32 fb, int32 r, int32 g, int32 b, int32 cycles) {
	// fx_generic_fade(mode,onOff,0,0,0,r,g,b,cycles
	// note from colour is always <0,0,0>
	int32 p[9] = {mode, toFrom, fr, fg, fb, r, g, b, cycles};
	int32 ret;
	return MS->fx_generic_fade(ret, p);
}

// take <r,g,b> and cycles
mcodeFunctionReturnCodes fx_brighten_to(int32 &, int32 *p) { return _simple_fx(0, 1, 0, 0, 0, p[0], p[1], p[2], p[3]); }

mcodeFunctionReturnCodes fx_brighten_from(int32 &, int32 *p) { return _simple_fx(0, 0, 0, 0, 0, p[0], p[1], p[2], p[3]); }

mcodeFunctionReturnCodes fx_darken_to(int32 &, int32 *p) { return _simple_fx(1, 1, 0, 0, 0, p[0], p[1], p[2], p[3]); }

mcodeFunctionReturnCodes fx_darken_from(int32 &, int32 *p) { return _simple_fx(1, 0, 0, 0, 0, p[0], p[1], p[2], p[3]); }

mcodeFunctionReturnCodes fx_fade_to(int32 &, int32 *p) { return _simple_fx(2, 1, 0, 0, 0, p[0], p[1], p[2], p[3]); }

mcodeFunctionReturnCodes fx_fade_from(int32 &, int32 *p) { return _simple_fx(2, 0, 0, 0, 0, p[0], p[1], p[2], p[3]); }

// blend function takes <r,g,b> -> <r,g,b> and cycles
mcodeFunctionReturnCodes fx_blend(int32 &, int32 *p) { return _simple_fx(3, 1, p[0], p[1], p[2], p[3], p[4], p[5], p[6]); }

} // End of namespace ICB
