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

#ifndef NUVIE_PATHFINDER_DIR_FINDER_H
#define NUVIE_PATHFINDER_DIR_FINDER_H

namespace Ultima {
namespace Nuvie {

class MapCoord;

class DirFinder {
public:
	DirFinder() { }

	static void get_adjacent_dir(sint8 &xdir, sint8 &ydir, sint8 rotate);
	static uint8 get_nuvie_dir(sint16 xrel, sint16 yrel);
	static uint8 get_nuvie_dir(uint16 sx, uint16 sy, uint16 tx, uint16 ty, uint8 z);
	static sint8 get_turn_towards_dir(sint16 oxdir, sint16 oydir, sint8 txdir, sint8 tydir);
	static void get_normalized_dir(MapCoord from, MapCoord to, sint8 &xdir, sint8 &ydir);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
