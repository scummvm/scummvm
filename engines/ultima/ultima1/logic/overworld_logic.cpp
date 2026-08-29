
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

#include "ultima/ultima1/logic/overworld_logic.h"
#include "ultima/ultima1/core/strings.h"
#include "ultima/ultima1/data/entity.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

enum {
	IMP_WATER, IMP_MOUNTAINS, IMP_WOODS, IMP_FRIGATES_WATER, IMP_RAFTS_WATER,
	IMP_BLOCKED_BY, IMP_MOVE_ON_LAND = 6
};

constexpr int OVERWORLD_MID_Y = Data::OVERWORLD_HEIGHT / 2 - 1;

static const char *IMPASSABLE_REASONS[] = {
	"You can't walk on water!",
	"Mountains are impassable!",
	"Aircars can't pass woods!",
	"Frigates like water!",
	"Rafts like water!"
};

const int8 DELTA_X[5] = { 0, -1, 1, 0, 0 };
const int8 DELTA_Y[5] = { 0, 0, 0, -1, 1 };

OverworldLogic::OverworldLogic() {
	_G(map)._mapType = Data::MAPTYPE_OVERWORLD;
}

void OverworldLogic::action(int action) {
	switch (action) {
	case KEYBIND_UP:
		move(Data::DIR_UP);
		break;
	case KEYBIND_DOWN:
		move(Data::DIR_DOWN);
		break;
	case KEYBIND_LEFT:
		move(Data::DIR_LEFT);
		break;
	case KEYBIND_RIGHT:
		move(Data::DIR_RIGHT);
		break;
	case KEYBIND_ENTER:
		enter();
		break;
	case KEYBIND_PASS:
		pass();
		break;
	default:
		writeString("Huh?\n");
		break;
	}

	endOfTurn();
}

void OverworldLogic::keypress(Common::KeyCode keycode) {
	writeString("Huh?\n");
	endOfTurn();
}

void OverworldLogic::enter() {
	int location = _G(map).getLocationAt(_G(savegame)._overworldPos);
	if (location == 0) {
		writeString("Enter?\n");
		playFX(1);
	} else {
		// Check for tiles surrounding the location
		_G(map)._waterCount = 0;
		_G(map)._grassCount = 0;
		_G(map)._woodsCount = 0;

		for (int deltaY = -1; deltaY <= 1; ++deltaY) {
			for (int deltaX = -1; deltaX <= 1; ++deltaX) {
				int tile = _G(map).getTileAt(_G(savegame)._overworldPos.x + deltaX, _G(savegame)._overworldPos.y + deltaY);
				switch (tile) {
				case Data::TILE_OCEAN:
					_G(map)._waterCount++;
					break;
				case Data::TILE_GRASS:
					_G(map)._grassCount++;
					break;
				case Data::TILE_WOODS:
					_G(map)._woodsCount++;
					break;
				default:
					break;
				}
			}
		}

		// Display the enter message
		_G(map)._isLordBritishCastle = location == 33;

		Common::String msg = "Entering...\n";
		if (!_G(map)._isLordBritishCastle)
			msg += "The city of ";
		msg += Data::LOCATION_NAMES[location - 1];
		msg += "\n";
		writeString(msg);

		// Load the new location
		_G(map).load(location);

		// Run any logic for entering it
		_G(logic)->enter();
	}
}

void OverworldLogic::pass() {
	writeString("Pass\n");
}

void OverworldLogic::move(Data::Direction dir) {
	if (!moveCheck(dir))
		return;

	if (_G(savegame)._transportType <= Data::TRANSPORT_CART)
		playFX(4);

	// Write direction traveled
	writeString(Common::String::format("%s\n", Data::DIRECTION_NAMES[dir]));

	int continentNum = _G(map).getContinentAt(_G(savegame)._overworldPos);

	switch (dir) {
	case Data::DIR_LEFT:
		moveLeft();
		break;
	case Data::DIR_RIGHT:
		moveRight();
		break;
	case Data::DIR_UP:
		moveUp();
		break;
	case Data::DIR_DOWN:
		moveDown();
		break;
	}

	if (_G(map).getContinentAt(_G(savegame)._overworldPos) != continentNum)
		continentChanged(continentNum);
}

bool OverworldLogic::moveCheck(Data::Direction dir) {
	int xp = _G(savegame)._overworldPos.x + DELTA_X[dir];
	int yp = _G(savegame)._overworldPos.y + DELTA_Y[dir];
	int tile = _G(map).getTileAt(xp, yp);
	int transport = _G(savegame)._transportType;

	if (tile >= Data::TILE_FIRST_MONSTER && tile <= Data::TILE_LAST_MONSTER) {
		Common::String msg = Common::String::format("Blocked by a");
		const char *monsterName = Data::OVERWORLD_MONSTERS[(tile - Data::TILE_FIRST_MONSTER) / 2];
		if (isVowel(*monsterName))
			msg += '\n';
		msg += ' ';

		if (tile == Data::TILE_WANDERING_WARLOCK)
			msg += '\n';
		msg += monsterName;
		msg += "!\n";

		writeString(msg);
		playFX(0);
		return false;

	} else if (tile == Data::TILE_MOUNTAINS) {
		impassable(IMP_MOUNTAINS);
		return false;

	} else if (tile == Data::TILE_OCEAN) {
		if (transport != Data::TRANSPORT_FRIGATE && transport != Data::TRANSPORT_RAFT &&
				transport != Data::TRANSPORT_AIRCAR) {
			impassable(IMP_WATER);
			return false;
		}
	} else if (tile == Data::TILE_WOODS && transport == Data::TRANSPORT_AIRCAR) {
		impassable(IMP_WOODS);
		return false;

	} else if (transport == Data::TRANSPORT_FRIGATE && tile != Data::TILE_OCEAN) {
		impassable(IMP_FRIGATES_WATER);
		return false;

	} else if (transport == Data::TRANSPORT_RAFT && tile != Data::TILE_OCEAN) {
		impassable(IMP_RAFTS_WATER);
		return false;

	} else if (transport >= Data::TRANSPORT_SHUTTLE) {
		impassable(IMP_MOVE_ON_LAND);
		return false;
	}

	return true;
}

int OverworldLogic::getViewportX(int xp) const {
	int x = _G(savegame)._overworldPos.x - Data::MAP_VISIBLE_CENTER_X + xp;
	if (x < 0)
		x += Data::OVERWORLD_WIDTH;
	else if (x >= Data::OVERWORLD_WIDTH)
		x -= Data::OVERWORLD_WIDTH;

	return x;
}

int OverworldLogic::getViewportY(int yp) const {
	int y = _G(savegame)._overworldPos.y - Data::MAP_VISIBLE_CENTER_Y + yp;
	if (y < 0)
		y += Data::OVERWORLD_HEIGHT;
	else if (y >= Data::OVERWORLD_HEIGHT)
		y -= Data::OVERWORLD_HEIGHT;

	return y;
}

void OverworldLogic::impassable(int reason) {
	writeString(Common::String::format("%s\n", IMPASSABLE_REASONS[reason]));
	playFX(0);
}

void OverworldLogic::moveLeft() {
	// Change overworld position
	Common::Point &pt = _G(savegame)._overworldPos;
	if (pt.x > 0) {
		pt.x--;
	} else {
		pt.x = Data::OVERWORLD_WIDTH - 1;
		pt.y = (pt.y >= OVERWORLD_MID_Y) ? pt.y - OVERWORLD_MID_Y : pt.y + OVERWORLD_MID_Y;
	}

	// Shift the existing columns right by one, to make room for a new
	// column of tiles at the left edge
	auto &tiles = _G(map)._mapTilesId;
	for (int col = Data::MAP_VISIBLE_WIDTH - 2; col >= 0; --col) {
		for (int row = 0; row < Data::MAP_VISIBLE_HEIGHT; ++row)
			tiles[row][col + 1] = tiles[row][col];
	}

	// Fill in the new left-hand column
	for (int row = 0; row < Data::MAP_VISIBLE_HEIGHT; ++row) {
		int x = getViewportX(0);
		int y = getViewportY(row);
		tiles[row][0] = _G(map).getMapTile(x, y);
	}
}

void OverworldLogic::moveRight() {
	// Change overworld position
	Common::Point &pt = _G(savegame)._overworldPos;
	if (pt.x != Data::OVERWORLD_WIDTH - 1) {
		pt.x++;
	} else {
		pt.y = (pt.y >= OVERWORLD_MID_Y) ? pt.y - OVERWORLD_MID_Y : pt.y + OVERWORLD_MID_Y;
		pt.x = 0;
	}

	// Shift the existing columns left by one, to make room for a new
	// column of tiles at the right edge
	auto &tiles = _G(map)._mapTilesId;
	for (int col = 1; col < Data::MAP_VISIBLE_WIDTH; ++col) {
		for (int row = 0; row < Data::MAP_VISIBLE_HEIGHT; ++row)
			tiles[row][col - 1] = tiles[row][col];
	}

	// Fill in the new right-hand column
	for (int row = 0; row < Data::MAP_VISIBLE_HEIGHT; ++row) {
		int x = getViewportX(Data::MAP_VISIBLE_WIDTH - 1);
		int y = getViewportY(row);
		tiles[row][Data::MAP_VISIBLE_WIDTH - 1] = _G(map).getMapTile(x, y);
	}
}

void OverworldLogic::moveUp() {
	// Change overworld position
	Common::Point &pt = _G(savegame)._overworldPos;
	pt.y = (pt.y == 0) ? Data::OVERWORLD_HEIGHT - 1 : pt.y - 1;

	// Shift the existing rows down by one, to make room for a new
	// row of tiles at the top edge
	auto &tiles = _G(map)._mapTilesId;
	for (int row = Data::MAP_VISIBLE_HEIGHT - 2; row >= 0; --row) {
		for (int col = 0; col < Data::MAP_VISIBLE_WIDTH; ++col)
			tiles[row + 1][col] = tiles[row][col];
	}

	// Fill in the new top row
	for (int col = 0; col < Data::MAP_VISIBLE_WIDTH; ++col) {
		int x = getViewportX(col);
		int y = getViewportY(0);
		tiles[0][col] = _G(map).getMapTile(x, y);
	}
}

void OverworldLogic::moveDown() {
	// Change overworld position
	Common::Point &pt = _G(savegame)._overworldPos;
	pt.y = (pt.y == Data::OVERWORLD_HEIGHT - 1) ? 0 : pt.y + 1;

	// Shift the existing rows up by one, to make room for a new
	// row of tiles at the bottom edge
	auto &tiles = _G(map)._mapTilesId;
	for (int row = 1; row < Data::MAP_VISIBLE_HEIGHT; ++row) {
		for (int col = 0; col < Data::MAP_VISIBLE_WIDTH; ++col)
			tiles[row - 1][col] = tiles[row][col];
	}

	// Fill in the new bottom row
	for (int col = 0; col < Data::MAP_VISIBLE_WIDTH; ++col) {
		int x = getViewportX(col);
		int y = getViewportY(Data::MAP_VISIBLE_HEIGHT - 1);
		tiles[Data::MAP_VISIBLE_HEIGHT - 1][col] = _G(map).getMapTile(x, y);
	}
}

void OverworldLogic::continentChanged(int oldContinent) {
	for (int index = 1; index < _G(overworldEntityCount); ++index) {
		auto &e = _G(savegame)._overworldEntities[index];
		int continent = _G(map).getContinentAt(e._x, e._y);

		if (continent == oldContinent) {
			_G(savegame).removeCreatureAt(e._x, e._y);
			_G(creaturesCount)--;
		}
	}
}

void OverworldLogic::endOfTurn() {
	if (_G(creaturesCount) < Data::CREATURES_COUNT)
		generateCreatures();

	_G(moveCtr)++;
	updateCreatures();
	reduceFood();

	redrawMap();
	redrawStats();
	prompt();
}

void OverworldLogic::generateCreatures() {
	int randomVal = getRandomNumber(1, 255);
	// Character level (experience / 1000), clamped to the table's bounds
	int level = MIN(_G(savegame)._experience / 1000, 9);
	if (randomVal >= Data::MONSTER_THRESHOLDS_PER_LEVEL[level])
		return;

	// Pick a monster type (0-14), weighted so tougher monster slots are
	// rarer to roll
	int weight = getRandomNumber(1, 8) * 2;
	int typeIdx = getRandomNumber(0, 149) / 10;
	int tierVal = Data::OVERWORLD_MONSTERS_HITS[typeIdx];
	if (weight > tierVal)
		weight = tierVal;

	// The home terrain the monster type needs to spawn on
	int data;
	if (typeIdx < 4)
		data = Data::TILE_OCEAN;
	else if (typeIdx < 9)
		data = Data::TILE_WOODS;
	else
		data = Data::TILE_GRASS;

	int tile = Data::TILE_FIRST_MONSTER + typeIdx * 2;
	int hits = Data::OVERWORLD_MONSTERS_HITS[typeIdx] * 10 + (level + 1) + 1;

	// Search for a valid spawn position, randomly offset just outside the
	// visible viewport, on the monster type's home terrain
	int x = 0, y = 0, tileAtSpot = 0;
	bool rangeOk = false;
	do {
		int xOff = getRandomNumber(10, 15);
		if (getRandomNumber(1, 16) & 1)
			xOff = -xOff;
		int yOff = getRandomNumber(5, 15);
		if (getRandomNumber(1, 16) & 1)
			yOff = -yOff;

		x = getViewportX(xOff + Data::MAP_VISIBLE_CENTER_X);
		y = getViewportY(yOff + Data::MAP_VISIBLE_CENTER_Y);
		tileAtSpot = _G(map).getTileAt(x, y);

		rangeOk = _G(map).mapRangeCheckX(x) && _G(map).mapRangeCheckY(y);
	} while (tileAtSpot != data && !rangeOk);

	if (tileAtSpot != data || !rangeOk)
		return;

	// Add the new entity (addOverworldEntity)
	if (_G(overworldEntityCount) < Data::OVERWORLD_ENTITY_COUNT - 1) {
		++_G(overworldEntityCount);
		auto &e = _G(savegame)._overworldEntities[_G(overworldEntityCount)];
		e._type = tile;
		e._data = data;
		e._x = x;
		e._y = y;
		e._hits = hits;
	}

	++_G(creaturesCount);
}

void OverworldLogic::updateCreatures() {
	int attackIndexes[Data::CREATURES_COUNT];
	int attackCount = 0;

	for (int idx = 1; idx <= _G(overworldEntityCount); ++idx) {
		auto &e = _G(savegame)._overworldEntities[idx];

		if (e._type >= Data::TILE_FIRST_MONSTER && e._type <= Data::TILE_LAST_MONSTER) {
			int xDiff = _G(savegame)._overworldPos.x - e._x;
			int yDiff = _G(savegame)._overworldPos.y - e._y;

			if (Data::OverworldEntity::getMonsterAttackDistance(e._type, xDiff, yDiff) != 0) {
				if (attackCount < Data::CREATURES_COUNT)
					attackIndexes[attackCount] = idx;
				++attackCount;
			} else {
				// TODO: monster movement (monsterMoveCheck, monsterMoveCheckX/Y, monsterTransportCheck)
			}
		}
	}

	while (attackCount > 0) {
		--attackCount;
		if (_G(savegame)._hits <= 0)
			break;

		int idx = attackIndexes[attackCount];
		auto &e = _G(savegame)._overworldEntities[idx];
		int xDiff = _G(savegame)._overworldPos.x - e._x;
		int yDiff = _G(savegame)._overworldPos.y - e._y;

		// TODO: monsterAttack(idx, xDiff, yDiff, getMonsterAttackDistance(e._type, xDiff, yDiff))
		// - creature combat/damage resolution
		warning("TODO: xDiff=%d, yDiff=%d", xDiff, yDiff);
	}
}

void OverworldLogic::reduceFood() {

}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
