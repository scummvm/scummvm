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

#include "ultima/ultima4/map/map_tile.h"

namespace Ultima {
namespace Ultima4 {

Direction MapTile::getDirection() const {
	return getTileType()->directionForFrame(_frame);
}

bool MapTile::setDirection(Direction d) {
	// if we're already pointing the right direction, do nothing!
	if (getDirection() == d)
		return false;

	const Tile *type = getTileType();

	int new_frame = type->frameForDirection(d);
	if (new_frame != -1) {
		_frame = new_frame;
		return true;
	}
	return false;
}

const Tile *MapTile::getTileType() const {
	return g_tileSets->findTileById(_id);
}

} // End of namespace Ultima4
} // End of namespace Ultima
