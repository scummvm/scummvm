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

#include "ultima/ultima4/direction.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/utils.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Returns the opposite direction.
 */
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
    default: break;
    }

    ASSERT(0, "invalid direction: %d", dir);
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

/**
 * Returns the a mask containing the broadsides directions for a given direction.
 * For instance, dirGetBroadsidesDirs(DIR_NORTH) returns:
 * (MASK_DIR(DIR_WEST) | MASK_DIR(DIR_EAST))
 */
int dirGetBroadsidesDirs(Direction dir) {
    int dirmask = MASK_DIR_ALL;
    dirmask = DIR_REMOVE_FROM_MASK(dir, dirmask);
    dirmask = DIR_REMOVE_FROM_MASK(dirReverse(dir), dirmask);

    return dirmask;
}

/**
 * Returns a random direction from a provided mask of available
 * directions.
 */
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

/**
 * Normalizes the direction based on the orientation given
 * (if facing west, and 'up' is pressed, the 'up' is translated
 *  into DIR_NORTH -- this function tranlates that direction 
 *  to DIR_WEST, the correct direction in this case).
 */
Direction dirNormalize(Direction orientation, Direction dir) {
    Direction temp = orientation,
              realDir = dir;

    while (temp != DIR_NORTH) {        
        temp = dirRotateCW(temp);
        realDir = dirRotateCCW(realDir);
    }

    return realDir;
}

/**
 * Translates a keyboard code into a direction
 */
Direction keyToDirection(int key) {        
    switch (key) {
    case U4_UP: return DIR_NORTH;        
    case U4_DOWN: return DIR_SOUTH;
    case U4_LEFT: return DIR_WEST;        
    case U4_RIGHT: return DIR_EAST;    
    default: return DIR_NONE;
    }    
}

/**
 * Translates a direction into a keyboard code
 */
int directionToKey(Direction dir) {
    switch(dir) {    
    case DIR_WEST: return U4_LEFT;
    case DIR_NORTH: return U4_UP;
    case DIR_EAST: return U4_RIGHT;
    case DIR_SOUTH: return U4_DOWN;
    
    case DIR_NONE:
    case DIR_ADVANCE:
    case DIR_RETREAT:
    default: break;        
    }    

    ASSERT(0, "Invalid diration passed to directionToKey()");
    return 0;
}

} // End of namespace Ultima4
} // End of namespace Ultima
