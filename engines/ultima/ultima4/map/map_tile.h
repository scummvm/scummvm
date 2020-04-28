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

#ifndef ULTIMA4_MAP_MAP_TILE_H
#define ULTIMA4_MAP_MAP_TILE_H

#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/map/tile.h"

namespace Ultima {
namespace Ultima4 {

/**
 * A MapTile is a specific instance of a Tile.
 */
class MapTile {
public:
	MapTile() : _id(0), _frame(0), _freezeAnimation(false) {
	}
	MapTile(const TileId &i, byte f = 0) : _id(i), _frame(f), _freezeAnimation(false) {
	}
	MapTile(const MapTile &t) : _id(t._id), _frame(t._frame), _freezeAnimation(t._freezeAnimation) {
	}

	TileId getId() const {
		return _id;
	}
	byte getFrame() const {
		return _frame;
	}
	bool getFreezeAnimation() const {
		return _freezeAnimation;
	}

	bool operator==(const MapTile &m) const {
		return _id == m._id;
	}
	bool operator==(const TileId &i) const {
		return _id == i;
	}
	bool operator!=(const MapTile &m) const {
		return _id != m._id;
	}
	bool operator!=(const TileId &i) const {
		return _id != i;
	}
	bool operator<(const MapTile &m) const {
		return _id < m._id;    // for Std::less
	}

	/**
	 * MapTile Class Implementation
	 */
	Direction getDirection() const;
	bool setDirection(Direction d);

	const Tile *getTileType() const;

	// Properties
	TileId _id;
	byte _frame;
	bool _freezeAnimation;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
