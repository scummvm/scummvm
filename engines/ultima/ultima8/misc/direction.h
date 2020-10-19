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

#ifndef ULTIMA8_MISC_DIRECTION_H
#define ULTIMA8_MISC_DIRECTION_H

namespace Ultima {
namespace Ultima8 {

/*
 *  Directions:
 */
enum Direction {
	dir_north = 0,
	dir_nne = 1,
	dir_northeast = 2,
	dir_ene = 3,
	dir_east = 4,
	dir_ese = 5,
	dir_southeast = 6,
	dir_sse = 7,
	dir_south = 8,
	dir_ssw = 9,
	dir_southwest = 10,
	dir_wsw = 11,
	dir_west = 12,
	dir_wnw = 13,
	dir_northwest = 14,
	dir_nnw = 15,
	dir_current = 16,
	dir_invalid = 100
};

enum DirectionMode {
	dirmode_8dirs,
	dirmode_16dirs
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
