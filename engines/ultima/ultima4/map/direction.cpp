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

#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

Direction dirReverse(Direction dir) {
	switch (dir) {
	case DIR_NONE:
		return DIR_NONE;
	case DIR_WEST:
		return DIR_EAST;
	case DIR_NORTH:
		return DIR_SOUTH;
	case DIR_EAST:
		return DIR_WEST;
	case DIR_SOUTH:
		return DIR_NORTH;
	case DIR_ADVANCE:
	case DIR_RETREAT:
	default:
		break;
	}

	error("invalid direction: %d", dir);
	return DIR_NONE;
}

Direction dirFromMask(int dir_mask) {
	if (dir_mask & MASK_DIR_NORTH) return DIR_NORTH;
	else if (dir_mask & MASK_DIR_EAST) return DIR_EAST;
	else if (dir_mask & MASK_DIR_SOUTH) return DIR_SOUTH;
	else if (dir_mask & MASK_DIR_WEST) return DIR_WEST;
	return DIR_NONE;
}

Direction dirRotateCW(Direction dir) {
	dir = static_cast<Direction>(dir + 1);
	if (dir > DIR_SOUTH)
		dir = DIR_WEST;
	return dir;
}

Direction dirRotateCCW(Direction dir) {
	dir = static_cast<Direction>(dir - 1);
	if (dir < DIR_WEST)
		dir = DIR_SOUTH;
	return dir;
}

int dirGetBroadsidesDirs(Direction dir) {
	int dirmask = MASK_DIR_ALL;
	dirmask = DIR_REMOVE_FROM_MASK(dir, dirmask);
	dirmask = DIR_REMOVE_FROM_MASK(dirReverse(dir), dirmask);

	return dirmask;
}

Direction dirRandomDir(int valid_directions_mask) {
	int i, n;
	Direction d[4];

	n = 0;
	for (i = DIR_WEST; i <= DIR_SOUTH; i++) {
		if (DIR_IN_MASK(i, valid_directions_mask)) {
			d[n] = static_cast<Direction>(i);
			n++;
		}
	}

	if (n == 0)
		return DIR_NONE;

	return d[xu4_random(n)];
}

Direction dirNormalize(Direction orientation, Direction dir) {
	Direction temp = orientation,
	          realDir = dir;

	while (temp != DIR_NORTH) {
		temp = dirRotateCW(temp);
		realDir = dirRotateCCW(realDir);
	}

	return realDir;
}

Direction keyToDirection(int key) {
	switch (key) {
	case Common::KEYCODE_UP:
		return DIR_NORTH;
	case Common::KEYCODE_DOWN:
		return DIR_SOUTH;
	case Common::KEYCODE_LEFT:
		return DIR_WEST;
	case Common::KEYCODE_RIGHT:
		return DIR_EAST;
	default:
		return DIR_NONE;
	}
}

int directionToKey(Direction dir) {
	switch (dir) {
	case DIR_WEST:
		return Common::KEYCODE_LEFT;
	case DIR_NORTH:
		return Common::KEYCODE_UP;
	case DIR_EAST:
		return Common::KEYCODE_RIGHT;
	case DIR_SOUTH:
		return Common::KEYCODE_DOWN;

	case DIR_NONE:
	case DIR_ADVANCE:
	case DIR_RETREAT:
	default:
		break;
	}

	error("Invalid diration passed to directionToKey()");
	return 0;
}

} // End of namespace Ultima4
} // End of namespace Ultima
