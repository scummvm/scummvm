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
	dir_northeast = 1,
	dir_east = 2,
	dir_southeast = 3,
	dir_south = 4,
	dir_southwest = 5,
	dir_west = 6,
	dir_northwest = 7,
	dir_current = 16
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
