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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/map.h"

namespace Sherlock {

Map::Map(SherlockEngine *vm): _vm(vm) {
}

/**
 * Loads the list of points for locations on the map for each scene
 */
void Map::loadPoints(int count, const int *xList, const int *yList) {
	for (int idx = 0; idx < count; ++idx, ++xList, ++yList) {
		_points.push_back(Common::Point(*xList, *yList));
	}
}

/**
 * Show the map
 */
int Map::show() {
	return 0;
}

} // End of namespace Sherlock
