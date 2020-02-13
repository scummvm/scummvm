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

#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/maps/map_dungeon.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/file.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

Ultima1Map::Ultima1Map(Ultima1Game *game) : Shared::Maps::Map(),
		_mapType(MAP_UNKNOWN), _moveCounter(0) {
	Ultima1Map::clear();
	_mapCity = new MapCity(game, this);
	_mapCastle = new MapCastle(game, this);
	_mapDungeon = new MapDungeon(game, this);
	_mapOverworld = new MapOverworld(game, this);
}

Ultima1Map::~Ultima1Map() {
	delete _mapCity;
	delete _mapCastle;
	delete _mapDungeon;
	delete _mapOverworld;
}

void Ultima1Map::clear() {
	_mapType = MAP_UNKNOWN;
}

void Ultima1Map::load(Shared::Maps::MapId mapId) {
	// If we're leaving the overworld, update the cached copy of the position in the overworld
	if (_mapType == MAP_OVERWORLD)
		_worldPos = _mapArea->getPosition();

	Shared::Maps::Map::load(mapId);

	// Switch to the correct map area
	if (mapId == MAPID_OVERWORLD) {
		_mapType = MAP_OVERWORLD;
		_mapArea = _mapOverworld;
	} else if (mapId < 33) {
		_mapType = MAP_CITY;
		_mapArea = _mapCity;
	} else if (mapId < 41) {
		_mapType = MAP_CASTLE;
		_mapArea = _mapCastle;
	} else if (mapId < 49) {
		error("TODO: load Pillar");
	} else {
		_mapType = MAP_DUNGEON;
		_mapArea = _mapDungeon;
	}

	// Load the map
	_mapArea->load(mapId);
}

void Ultima1Map::synchronize(Common::Serializer &s) {
	Shared::Maps::Map::synchronize(s);
	if (_mapType != MAP_OVERWORLD) {
		if (s.isLoading())
			_mapOverworld->load(MAP_OVERWORLD);
		_mapOverworld->synchronize(s);
	}

	s.syncAsUint32LE(_moveCounter);
}

void Ultima1Map::dropCoins(uint amount) {
	static_cast<MapBase *>(_mapArea)->dropCoins(amount);
}

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
