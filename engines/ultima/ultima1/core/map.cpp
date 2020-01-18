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

#include "ultima/ultima1/core/map.h"
#include "ultima/ultima1/core/transports.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/file.h"

namespace Ultima {
namespace Ultima1 {

using Shared::File;

Ultima1Map::Ultima1Map(Ultima1Game *game) : Shared::Map(), _mapType(MAP_OVERWORLD), _mapStyle(0), _mapIndex(0) {
	_currentTransport = new TransportOnFoot(game, this);
	addWidget(_currentTransport);
}

void Ultima1Map::loadMap(int mapId, uint videoMode) {
	Map::loadMap(mapId, videoMode);

	if (mapId == 0)
		loadOverworldMap();
	else
		loadTownCastleMap();
}

void Ultima1Map::loadOverworldMap() {
	_mapType = MAP_OVERWORLD;
	_mapStyle = _mapIndex = 0;

	_size = Point(168, 156);
	_tilesPerOrigTile = Point(1, 1);
	_data.resize(_size.x * _size.y);

	File f("map.bin");
	byte b;
	for (int y = 0; y < _size.y; ++y) {
		for (int x = 0; x < _size.x; x += 2) {
			b = f.readByte();
			_data[y * _size.x + x] = b >> 4;
			_data[y * _size.x + x + 1] = b & 0xf;
		}
	}
}

void Ultima1Map::loadTownCastleMap() {
	_size = Point(38, 18);
	_tilesPerOrigTile = Point(1, 1);
	_data.resize(_size.x * _size.y);

	File f("tcd.bin");
	f.seek(_mapId * 684);
	for (int y = 0; y < _size.y; ++y) {
		for (int x = 0; x < _size.x; ++x)
			_data[y * _size.x + x] = f.readByte();
	}

	_fixed = true;
	if (_mapId < 33) {
		// Town/city
		_mapType = MAP_TOWN;
		_mapStyle = (_mapId % 8) + 2;
		_mapIndex = _mapId;
	} else {
		// Castle
		_mapType = MAP_CASTLE;
		_mapStyle = _mapId % 2;
		_mapIndex = _mapId - 33;
	}
}

void Ultima1Map::getTileAt(const Point &pt, U1MapTile *tile) {
	Shared::Map::getTileAt(pt, tile);

}

} // End of namespace Ultima1
} // End of namespace Ultima
