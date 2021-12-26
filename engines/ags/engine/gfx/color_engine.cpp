/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//=============================================================================
//
// Implementation from wgt2allg.cpp specific to Engine runtime
//
//=============================================================================

#include "ags/lib/allegro.h"
#include "ags/shared/util/wgt2_allg.h"

namespace AGS3 {

void __my_setcolor(int *ctset, int newcol, int wantColDep) {
	if (wantColDep == 8)
		ctset[0] = newcol;
	else if (newcol & 0x40000000) // already calculated it
		ctset[0] = newcol;
	else if ((newcol >= 32) && (wantColDep > 16)) {
		// true-color
#ifdef SWAP_RB_HICOL_FOR_32to24_32
		ctset[0] = makeacol32(getb16(newcol), getg16(newcol), getr16(newcol), 255);
#else
		ctset[0] = makeacol32(getr16(newcol), getg16(newcol), getb16(newcol), 255);
#endif
	} else if (newcol >= 32) {

		// If it's 15-bit, convert the color
		if (wantColDep == 15)
			ctset[0] = (newcol & 0x001f) | ((newcol >> 1) & 0x7fe0);
		else
			ctset[0] = newcol;
	} else {
		ctset[0] = makecol_depth(wantColDep, col_lookups[newcol] >> 16,
		                         (col_lookups[newcol] >> 8) & 0x000ff, col_lookups[newcol] & 0x000ff);

		// in case it's used on an alpha-channel sprite, make sure it's visible
		if (wantColDep > 16)
			ctset[0] |= 0xff000000;
	}

	// if it's 32-bit color, signify that the colour has been calculated
	//if (wantColDep >= 24)
//      ctset[0] |= 0x40000000;
}

} // namespace AGS3
