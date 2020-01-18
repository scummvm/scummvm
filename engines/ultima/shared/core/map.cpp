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

#include "ultima/shared/core/map.h"

namespace Ultima {
namespace Shared {

Map::Map() {
	_mapId = 0;
	_mapType = MAP_OVERWORLD;
	_mapStyle = 0;
	_direction = DIR_NORTH;
	_fixed = false;
}

Common::Point Map::getRelativePosition(const Common::Point &delta) {
	Common::Point pt = _currentPos + delta;
	if (pt.x < 0)
		pt.x += _size.x;
	else if (pt.x >= _size.x)
		pt.x -= _size.x;
	if (pt.y < 0)
		pt.y += _size.y;
	else if (pt.y >= _size.y)
		pt.y -= _size.y;

	return pt;
}

void Map::loadMap(int mapId, uint videoMode) {
	_mapId = mapId;
	_mapType = MAP_OVERWORLD;
	_mapStyle = 0;
	_fixed = false;
}

void Map::setPosition(const Common::Point &pt) {
	_currentPos = Common::Point(pt.x * _tilesPerOrigTile.x, pt.y * _tilesPerOrigTile.y);
}

} // End of namespace Shared
} // End of namespace Ultima
