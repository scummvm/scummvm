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

#ifndef ICB_PRIM_ROUTE_BUILDER_H
#define ICB_PRIM_ROUTE_BUILDER_H

#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/route_manager.h"

namespace ICB {

#define MAX_barriers 255 // Changed from 256 to 300, 'cos M08 was tipping it.

#if MAX_barriers > 255

#error "MAX_barriers must be less than 256, because of e.g. uint8 temp_route[MAX_final_route];"

#endif

enum _route_stat { __PRIM_ROUTE_OK, __PRIM_ROUTE_FAIL };

class _prim_route_builder {

 public:
	void Reset_barrier_list();
	void Add_barrier(_route_barrier *new_barrier);

	_route_stat Calc_route(PXreal startx, PXreal startz, PXreal endx, PXreal endz); // does extraping

	void Give_route(_route_description *route); // copy the route to pass_to
	void Give_barrier_list(_route_description *route); // copy the barriers - this is for the debug NETHACK diagnostics mode

	uint32 final_points;
	_point final_route[MAX_final_route]; // tempory place for final route - caller should copy out
	PXreal final_len; // internal temp flag - here so we can print it

	uint32 extrap_size; // denotes the length to extend barriers by

	// Public so everyone can use this function
	int32 Get_intersect(PXreal x0, PXreal y0, PXreal x1, PXreal y1, PXreal x2, PXreal y2, PXreal x3, PXreal y3);

	// This is a wrapper function to make the Cohen-Sutherland line-intersection algorithm
	// available to the router.
	bool8 LineIntersectsRect(DXrect oRect, int32 nX1, int32 nY1, int32 nX2, int32 nY2) const;

private:
	_point barrier_list[MAX_barriers];

	uint32 total_points; // points dervived from start and end of each barrier line

	// Bit based arrays hence the add 7 and divide by 8 (+7 rounds up)
	uint8 hitsBits[MAX_barriers][(MAX_barriers + 7) / 8];
	uint8 gohitsBits[MAX_barriers][(MAX_barriers + 7) / 8];

	// Jake : note this can only be uint8 if MAX_barriers < 256
	uint8 temp_route[MAX_final_route];

	// These could be bit arrays as well :
	// but not much gain compared to hits & gohits
	uint8 exclude[MAX_barriers];
	uint8 temp_exclude[MAX_barriers];

#if PRIM_BYTE_ARRAYS
	uint8 hits[MAX_barriers][MAX_barriers];
	uint8 gohits[MAX_barriers][MAX_barriers];
#endif

	void Find_connects(uint32 point, PXreal cur_len, uint32 level);
	uint32 ExtrapolateLine(_point *pSrc0, _point *pSrc1, _point *pDst0, _point *pDst1, int32 d);
	PXfloat m_fabs(PXfloat val); // abs
};

} // End of namespace ICB

#endif
