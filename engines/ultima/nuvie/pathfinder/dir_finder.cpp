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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/pathfinder/dir_finder.h"

namespace Ultima {
namespace Nuvie {

/** STATIC FUNCTIONS **/

/* From a normalized direction xdir,ydir as base, get the normalized direction
 * towards one of the seven nearby tiles to the left or right of base.
 * For rotate: -n = left n tiles, n = right n tiles
 */
void DirFinder::get_adjacent_dir(sint8 &xdir, sint8 &ydir, sint8 rotate) {
	struct {
		sint8 x, y;
	} neighbors[8] = { { -1, -1}, { +0, -1}, { +1, -1},
		{ +1, +0},/*ACTOR*/ { +1, +1},
		{ +0, +1}, { -1, +1}, { -1, +0}
	};

	for (uint32 start = 0; start < 8; start++)
		if (neighbors[start].x == xdir && neighbors[start].y == ydir) {
			sint32 dest = start + rotate;
			while (dest < 0 || dest > 7)
				dest += (dest < 0) ? 8 : -8;
			xdir = neighbors[dest].x;
			ydir = neighbors[dest].y;
			break;
		}
}

uint8 DirFinder::get_nuvie_dir(sint16 xrel, sint16 yrel) {
	uint8 direction = NUVIE_DIR_N; // default

	if (xrel == 0 && yrel == 0) // nowhere
		return (direction);
	if (xrel == 0) // up or down
		direction = (yrel < 0) ? NUVIE_DIR_N : NUVIE_DIR_S;
	else if (yrel == 0) // left or right
		direction = (xrel < 0) ? NUVIE_DIR_W : NUVIE_DIR_E;
	else if (xrel < 0 && yrel < 0)
		direction = NUVIE_DIR_NW;
	else if (xrel > 0 && yrel < 0)
		direction = NUVIE_DIR_NE;
	else if (xrel < 0 && yrel > 0)
		direction = NUVIE_DIR_SW;
	else if (xrel > 0 && yrel > 0)
		direction = NUVIE_DIR_SE;
	return (direction);
}

uint8 DirFinder::get_nuvie_dir(uint16 sx, uint16 sy, uint16 tx, uint16 ty, uint8 z) {
	return DirFinder::get_nuvie_dir(get_wrapped_rel_dir(tx, sx, z), get_wrapped_rel_dir(ty, sy, z));
}

// oxdir = original xdir, txdir = to xdir
sint8 DirFinder::get_turn_towards_dir(sint16 oxdir, sint16 oydir, sint8 txdir, sint8 tydir) {
	oxdir = clamp(oxdir, -1, 1);
	oydir = clamp(oydir, -1, 1);
	txdir = clamp(txdir, -1, 1);
	tydir = clamp(tydir, -1, 1);
	struct {
		sint8 x, y;
	} dirs[8] = {{ -1, -1}, { +0, -1}, { +1, -1}, { +1, +0}, { +1, +1}, { +0, +1}, { -1, +1}, { -1, +0}};
	uint8 t = 0, o = 0;
	for (uint8 d = 0; d < 8; d++) {
		if (dirs[d].x == oxdir && dirs[d].y == oydir)
			o = d;
		if (dirs[d].x == txdir && dirs[d].y == tydir)
			t = d;
	}
	sint8 turn = t - o;
	if (turn > 4)
		turn = -(8 - turn);
	return (clamp(turn, -1, 1));
}

// xdir,ydir = normal direction from->to (simple method)
void DirFinder::get_normalized_dir(MapCoord from, MapCoord to, sint8 &xdir, sint8 &ydir) {
	xdir = clamp(to.x - from.x, -1, 1);
	ydir = clamp(to.y - from.y, -1, 1);

	/*    uint16 dx = from.xdistance(to), dy = from.ydistance(to);
	    if(dx > 0 && dy > 0)
	    {
	        if(dx > dy) xdir = 0;
	        else if(dx < dy) ydir = 0;
	    }*/
}

} // End of namespace Nuvie
} // End of namespace Ultima
