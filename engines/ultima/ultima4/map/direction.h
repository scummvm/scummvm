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

#ifndef ULTIMA4_MAP_DIRECTION_H
#define ULTIMA4_MAP_DIRECTION_H

namespace Ultima {
namespace Ultima4 {

enum Direction {
	DIR_NONE,
	DIR_WEST,
	DIR_NORTH,
	DIR_EAST,
	DIR_SOUTH,
	DIR_ADVANCE,
	DIR_RETREAT
};

#define MASK_DIR(dir) (1 << (dir))
#define MASK_DIR_WEST (1 << DIR_WEST)
#define MASK_DIR_NORTH (1 << DIR_NORTH)
#define MASK_DIR_EAST (1 << DIR_EAST)
#define MASK_DIR_SOUTH (1 << DIR_SOUTH)
#define MASK_DIR_ADVANCE (1 << DIR_ADVANCE)
#define MASK_DIR_RETREAT (1 << DIR_RETREAT)
#define MASK_DIR_ALL (MASK_DIR_WEST | MASK_DIR_NORTH | MASK_DIR_EAST | MASK_DIR_EAST | MASK_DIR_SOUTH | MASK_DIR_ADVANCE | MASK_DIR_RETREAT)

#define DIR_IN_MASK(dir,mask) ((1 << (dir)) & (mask))
#define DIR_ADD_TO_MASK(dir,mask) ((1 << (dir)) | (mask))
#define DIR_REMOVE_FROM_MASK(dir,mask) ((~(1 << (dir))) & (mask))

/**
 * Returns the opposite direction.
 */
Direction dirReverse(Direction dir);

Direction dirFromMask(int dir_mask);
Direction dirRotateCW(Direction dir);
Direction dirRotateCCW(Direction dir);

/**
 * Returns the a mask containing the broadsides directions for a given direction.
 * For instance, dirGetBroadsidesDirs(DIR_NORTH) returns:
 * (MASK_DIR(DIR_WEST) | MASK_DIR(DIR_EAST))
 */
int dirGetBroadsidesDirs(Direction dir);

/**
 * Returns a random direction from a provided mask of available
 * directions.
 */
Direction dirRandomDir(int valid_directions_mask);

/**
 * Normalizes the direction based on the orientation given
 * (if facing west, and 'up' is pressed, the 'up' is translated
 *  into DIR_NORTH -- this function tranlates that direction
 *  to DIR_WEST, the correct direction in this case).
 */
Direction dirNormalize(Direction orientation, Direction dir);

/**
 * Translates a keyboard code into a direction
 */
Direction keyToDirection(int key);

/**
 * Translates a direction into a keyboard code
 */
int directionToKey(Direction dir);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
