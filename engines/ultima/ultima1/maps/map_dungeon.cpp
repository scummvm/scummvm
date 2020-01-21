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

#include "ultima/ultima1/maps/map_dungeon.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/widgets/dungeon_widget.h"
#include "ultima/ultima1/widgets/dungeon_monster.h"
#include "ultima/ultima1/widgets/dungeon_player.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

void MapDungeon::load(Shared::Maps::MapId mapId) {
	Shared::Maps::MapBase::load(mapId);

	_tilesPerOrigTile = Point(1, 1);
	_dungeonLevel = 1;

	changeLevel(0);
	_playerWidget->moveTo(Point(1, 1), Shared::Maps::DIR_SOUTH);
}

bool MapDungeon::changeLevel(int delta) {
	_dungeonLevel += delta;
	if (_dungeonLevel <= 0)
		return false;

	// Set seed for generating a deterministic resulting dungoen level
	setRandomSeed();

	// Reset dungeon area
	setDimensions(Point(DUNGEON_WIDTH, DUNGEON_HEIGHT));

	if (_widgets.empty()) {
		// Set up widget for the player
		_playerWidget = new Widgets::DungeonPlayer(_game, this);
		addWidget(_playerWidget);
	} else {
		_widgets.resize(1);
	}

	// Place walls around the edge of the map
	for (int y = 0; y < DUNGEON_HEIGHT; ++y) {
		_data[y][0] = DTILE_WALL;
		_data[y][DUNGEON_WIDTH - 1] = DTILE_WALL;
	}
	for (int x = 0; x < DUNGEON_WIDTH; ++x) {
		_data[0][x] = DTILE_WALL;
		_data[DUNGEON_HEIGHT - 1][x] = DTILE_WALL;
	}

	// Set up walls vertically across the dungeon
	for (int x = 2; x < (DUNGEON_WIDTH - 1); x += 2)
		for (int y = 2; y < (DUNGEON_HEIGHT - 1); y += 2)
			_data[y][x] = DTILE_WALL;

	// Randomly set up random tiles for all alternate positions in wall columns
	for (int x = 2; x < (DUNGEON_WIDTH - 1); x += 2)
		for (int y = 1; y < DUNGEON_HEIGHT; y += 2)
			_data[y][x] = getDeterministicRandomNumber(DTILE_HALLWAY, DTILE_DOOR);

	// Set up wall and beams randomly to subdivide the blank columns
	const byte DATA1[15] = { 8, 5, 2, 8, 1, 5, 4, 6, 1, 3, 7, 3, 9, 2, 6 };
	const byte DATA2[15] = { 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 8, 8, 9, 9 };
	for (uint ctr = 0; ctr < (_dungeonLevel * 2); ++ctr) {
		byte newTile = (getDeterministicRandomNumber(0, 255) <= 160) ? DTILE_WALL : DTILE_BEAMS;
		uint idx = getDeterministicRandomNumber(0, 14);

		if (_dungeonLevel & 1) {
			_data[DATA2[idx]][DATA1[idx]] = newTile;
		}
		else {
			_data[DATA1[idx]][DATA2[idx]] = newTile;
		}
	}

	// Place chests and/or coffins randomly throughout the level
	_random.setSeed(_random.getSeed() + 1777);
	for (uint ctr = 0; ctr <= _dungeonLevel; ++ctr) {
		Point pt(getDeterministicRandomNumber(10, 99) / 10, getDeterministicRandomNumber(10, 99) / 10);
		byte currTile = _data[pt.y][pt.x];

		if (currTile != DTILE_WALL && currTile != DTILE_SECRET_DOOR && currTile != DTILE_BEAMS) {
			_widgets.push_back(Shared::Maps::MapWidgetPtr(new Widgets::DungeonWidget(_game, this, pt,
				(getDeterministicRandomNumber(1, 100) & 1) ? Widgets::DITEM_COFFIN : Widgets::DITEM_CHEST)));
		}
	}

	// Set up ladders
	_data[2][1] = DTILE_HALLWAY;
	if (_dungeonLevel & 1) {
		_data[3][7] = DTILE_LADDER_UP;
		_data[6][3] = DTILE_LADDER_DOWN;
	} else {
		_data[3][7] = DTILE_LADDER_DOWN;
		_data[6][3] = DTILE_LADDER_UP;
	}

	if (_dungeonLevel == 10)
		_data[3][7] = DTILE_HALLWAY;
	if (_dungeonLevel == 1) {
		_data[1][1] = DTILE_LADDER_UP;
		_data[3][7] = DTILE_HALLWAY;
	}

	for (int ctr = 0; ctr < 3; ++ctr)
		spawnMonster();

	return true;
}


void MapDungeon::setRandomSeed() {
	Ultima1Map *map = static_cast<Ultima1Map *>(_game->getMap());
	uint32 seed = _game->_randomSeed + map->_worldPos.x * 5 + map->_worldPos.y * 3 + _dungeonLevel * 17;
	_random.setSeed(seed);
}

void MapDungeon::spawnMonster() {
	U1MapTile tile;

	// Pick a random position for the monster, trying again up to 500 times
	// if the chosen position isn't a valid place for the monster
	for (int tryNum = 0; tryNum < 500; ++tryNum) {
		Point newPos(_game->getRandomNumber(242) % 9 + 1, _game->getRandomNumber(242) % 9 + 1);
		getTileAt(newPos, &tile);

		if (tile._tileNum == 0 && tile._widgetNum == -1) {
			// Found a free spot
			spawnMonsterAt(newPos);
			break;
		}
	}
}

void MapDungeon::spawnMonsterAt(const Point &pt) {
	// Try up 50 times to randomly pick a monster not already present in the dungeon map
	for (int tryNum = 0; tryNum < 50; ++tryNum) {
		Widgets::DungeonWidgetId monsterId = (Widgets::DungeonWidgetId)((_dungeonLevel - 1) / 2 * 5 + _game->getRandomNumber(4));

		// Only allow one of every type of monster on the map at the same time
		uint monsIdx;
		for (monsIdx = 0; monsIdx < _widgets.size(); ++monsIdx) {
			Widgets::DungeonMonster *mons = dynamic_cast<Widgets::DungeonMonster *>(_widgets[monsIdx].get());
			if (mons && mons->id() == monsterId)
				break;
		}

		if (monsIdx == _widgets.size()) {
			// Monster not present, so can be added
			uint hp = _game->getRandomNumber(1, _dungeonLevel * _dungeonLevel + 1) +
				(int)monsterId + 10;
			Widgets::DungeonMonster *monster = new Widgets::DungeonMonster(_game, this, monsterId, hp, pt);
			addWidget(monster);
			return;
		}
	}
}

void MapDungeon::update() {
	U1MapTile tile;
	Point pt;

	// Widgets in the dungeon are updated by row
	for (pt.y = 1; pt.y < ((int)height() - 1) && !_game->_party.isFoodless(); pt.y++) {
		for (pt.x = 1; pt.x < ((int)width() - 1); pt.x++) {
			// Check for a widget at the given position
			getTileAt(pt, &tile);

			Shared::Maps::Creature *creature = dynamic_cast<Shared::Maps::Creature *>(tile._widget);
			if (creature)
				creature->update(true);
		}
	}
}

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
