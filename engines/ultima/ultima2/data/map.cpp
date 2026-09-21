/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "ultima/ultima2/data/map.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/logic/overworld_logic.h"
#include "ultima/ultima2/logic/city_castle_logic.h"
#include "ultima/ultima2/logic/dungeon_logic.h"
#include "ultima/shared/gfx/view.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

// The other planets' files are named with a G rather than the usual X
static char filePrefix() {
	return _G(savegame)._orbitTarget == 0 ? 'X' : 'G';
}

Common::String mapFilename(int mapEra, int mapType) {
	return Common::String::format("MAP%c%c%c", filePrefix(), '0' + mapEra, '0' + mapType);
}

Common::String monsterFilename(int mapEra, int mapType) {
	return Common::String::format("MON%c%c%c", filePrefix(), '0' + mapEra, '0' + mapType);
}

Common::String talkFilename(int mapEra, int mapType) {
	return Common::String::format("TLK%c%c%c", filePrefix(), '0' + mapEra, '0' + mapType);
}

void Map::loadTalk(int mapEra, int mapType) {
	_talk.clear();

	Common::File f;
	if (!f.open(talkFilename(mapEra, mapType).c_str()))
		return;

	// Zero-terminated strings with the high bit set on every character,
	// after a leading zero, so line 1 is the first string
	byte buffer[256] = {};
	f.read(buffer, sizeof(buffer));

	Common::String line;
	_talk.push_back(line);
	for (int i = 1; i < 256; ++i) {
		if (buffer[i] == 0) {
			_talk.push_back(line);
			line.clear();
		} else {
			line += (char)(buffer[i] & 0x7F);
		}
	}

	for (uint i = 0; i < _talk.size(); ++i) {
		for (uint j = 0; j < _talk[i].size(); ++j) {
			if (_talk[i][j] == '\r')
				_talk[i].setChar('\n', j);
		}
	}
}

void Map::load(int mapEra, int mapType) {
	if (mapType >= 4) {
		loadDungeon(mapEra, mapType);
		return;
	}

	Common::File f;
	Common::String filename = mapFilename(mapEra, mapType);
	if (!f.open(filename.c_str()))
		error("Could not open %s", filename.c_str());

	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_tiles[y][x] = (TileId)(f.readByte() / 4);
	}

	_monsters.load(mapEra, mapType);
	loadTalk(mapEra, mapType);

	// The files store each monster's tile baked into the map; restore the
	// terrain underneath, since monsters are tracked and drawn separately
	for (int slot = 0; slot < MAP_MONSTER_COUNT; ++slot) {
		if (!_monsters.isActive(slot))
			continue;

		int x = _monsters._mapX[slot], y = _monsters._mapY[slot];
		if (x < MAP_WIDTH && y < MAP_HEIGHT && _tiles[y][x] == _monsters.tileType(slot))
			_tiles[y][x] = (TileId)(_monsters._glyphTile[slot] / 4);
	}

	if (mapType == 0)
		_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::OverworldLogic());
	else
		_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::CityCastleLogic());

	showMapView(mapType == 0 ? "OverworldMap" : "LocationMap");
}

void Map::synchronize(Common::Serializer &s, bool isDungeon) {
	_monsters.synchronize(s);

	if (isDungeon) {
		_G(dungeon).synchronize(s);
	} else {
		for (int y = 0; y < MAP_HEIGHT; ++y) {
			for (int x = 0; x < MAP_WIDTH; ++x) {
				byte tile = _tiles[y][x];
				s.syncAsByte(tile);
				_tiles[y][x] = (TileId)tile;
			}
		}
	}
}

void Map::loadDungeon(int mapEra, int mapType) {
	_G(dungeon).load(mapEra, mapType);
	_monsters.load(mapEra, mapType);
	_talk.clear();

	// Each dungeon monster is also flagged in the low bits of its cell
	for (int slot = 0; slot < MAP_MONSTER_COUNT; ++slot) {
		if (_monsters.isActive(slot)) {
			_G(dungeon).cell(_monsters._glyphTile[slot], _monsters._mapX[slot],
				_monsters._mapY[slot]) |= _monsters._type[slot] & 7;
		}
	}

	_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::DungeonLogic());
	showMapView("DungeonMap");
}

void Map::showMapView(const Common::String &mapViewName) {
	if (!g_engine->isPresent("Game")) {
		// Set up the game view and force it to draw immediately
		g_engine->replaceView("Game", true);
		g_engine->focusedView()->draw();

		// Now open the map view on top of it
		g_engine->addView(mapViewName);
	} else {
		// Just switching from one map view to another
		g_engine->replaceView(mapViewName);
	}
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
