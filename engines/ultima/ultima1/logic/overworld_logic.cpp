
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

#include "common/stack.h"
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

static const int8 DELTA_X[5] = { 0, -1, 1, 0, 0 };
static const int8 DELTA_Y[5] = { 0, 0, 0, -1, 1 };

OverworldLogic::OverworldLogic() {
	_G(map)._mapType = Data::MAPTYPE_OVERWORLD;
}

bool OverworldLogic::enter() {
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
		_G(logic)->entering();
	}

	return true;
}

bool OverworldLogic::move(Data::Direction dir) {
	if (!moveCheck(dir))
		return true;

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

	return true;
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
}

void OverworldLogic::moveUp() {
	// Change overworld position
	Common::Point &pt = _G(savegame)._overworldPos;
	pt.y = (pt.y == 0) ? Data::OVERWORLD_HEIGHT - 1 : pt.y - 1;
}

void OverworldLogic::moveDown() {
	// Change overworld position
	Common::Point &pt = _G(savegame)._overworldPos;
	pt.y = (pt.y == Data::OVERWORLD_HEIGHT - 1) ? 0 : pt.y + 1;
}

void OverworldLogic::continentChanged(int oldContinent) {
	for (int index = 1; index < _G(overworldEntityCount); ++index) {
		auto &e = _G(savegame)._overworldEntities[index];
		int continent = _G(map).getContinentAt(e._x, e._y);

		if (continent == oldContinent) {
			_G(savegame).removeOverworldCreatureAt(e._x, e._y);
			_G(creaturesCount)--;
		}
	}
}

void OverworldLogic::endOfTurn() {
	if (_G(creaturesCount) < Data::CREATURES_COUNT)
		generateCreatures();
	updateCreatures();
	reduceFood();

	Logic::endOfTurn();
}

void OverworldLogic::generateCreatures() {
	const Common::Point &pos = _G(savegame)._overworldPos;
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

		x = pos.x + xOff;	// getViewportX(xOff + Data::OVERWORLD_VISIBLE_CENTER_X);
		y = pos.y + yOff;	// getViewportY(yOff + Data::OVERWORLD_VISIBLE_CENTER_Y);
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
	Common::Stack<int> attackers;

	for (int idx = 1; idx <= _G(overworldEntityCount); ++idx) {
		auto &e = _G(savegame)._overworldEntities[idx];

		if (e._type >= Data::TILE_FIRST_MONSTER && e._type <= Data::TILE_LAST_MONSTER) {
			int xDiff = _G(savegame)._overworldPos.x - e._x;
			int yDiff = _G(savegame)._overworldPos.y - e._y;
			int distance = Data::OverworldEntity::getMonsterAttackDistance(e._type, xDiff, yDiff);

			if (distance != 0) {
				if ((int)attackers.size() < Data::CREATURES_COUNT)
					attackers.push(idx);
			} else {
				monsterMoveCheck(idx, xDiff, yDiff);
			}
		}
	}

	while (!attackers.empty() && _G(savegame)._hits > 0) {
		// Get the attacker index from the end of the 
		int idx = attackers.pop();

		auto &e = _G(savegame)._overworldEntities[idx];
		int xDiff = _G(savegame)._overworldPos.x - e._x;
		int yDiff = _G(savegame)._overworldPos.y - e._y;
		int distance = Data::OverworldEntity::getMonsterAttackDistance(e._type, xDiff, yDiff);

		monsterAttack(idx, xDiff, yDiff, distance);
	}
}

void OverworldLogic::reduceFood() {

}

bool OverworldLogic::monsterTransportCheck(int entityIndex, int xDiff, int yDiff) {
	// Faster player transports make it harder for monsters to keep up - each
	// movement attempt has a per-transport-type chance of being skipped
	if (getRandomNumber(1, 100) > Data::TRANSPORT_RANDOM_THRESHOLD[_G(savegame)._transportType])
		return false;

	const auto &e = _G(savegame)._overworldEntities[entityIndex];
	int tile1, tile2;
	if (e._type >= Data::TILE_FIRST_MONSTER && e._type <= 25) {
		// Sea monsters can only move onto water
		tile1 = tile2 = Data::TILE_OCEAN;
	} else if (e._type == 31 || e._type == 35) {
		// These two monster types can only move onto woods
		tile1 = tile2 = Data::TILE_WOODS;
	} else {
		// Everything else can move onto either woods or grass
		tile1 = Data::TILE_WOODS;
		tile2 = Data::TILE_GRASS;
	}

	int newX = _G(savegame)._overworldPos.x - xDiff;
	int newY = _G(savegame)._overworldPos.y - yDiff;
	int tile = _G(map).getTileAt(newX, newY);

	// Single-terrain (amphibious) monster types are also allowed to move
	// onto a city/castle/dungeon entrance tile, regardless of the mismatch
	if (tile1 == tile2 && tile >= Data::TILE_CASTLE1 && tile <= Data::TILE_DUNGEON)
		return true;

	return (tile == tile1 || tile == tile2) &&
		_G(map).mapRangeCheckX(newX) && _G(map).mapRangeCheckY(newY);
}

bool OverworldLogic::monsterMoveCheckX(int entityIndex, int xDiff, int yDiff) {
	if (xDiff == 0)
		return false;

	int step = (xDiff > 0) ? -1 : 1;
	if (!monsterTransportCheck(entityIndex, xDiff + step, yDiff))
		return false;

	_G(savegame)._overworldEntities[entityIndex]._x -= step;
	return true;
}

bool OverworldLogic::monsterMoveCheckY(int entityIndex, int xDiff, int yDiff) {
	if (yDiff == 0)
		return false;

	int step = (yDiff > 0) ? -1 : 1;
	if (!monsterTransportCheck(entityIndex, xDiff, yDiff + step))
		return false;

	_G(savegame)._overworldEntities[entityIndex]._y -= step;
	return true;
}

void OverworldLogic::monsterMoveCheck(int entityIndex, int xDiff, int yDiff) {
	// Randomly vary which axis is tried first, so pursuing monsters don't
	// always prefer the same axis when moving diagonally
	if (getRandomNumber(1, 10) & 1) {
		if (!monsterMoveCheckY(entityIndex, xDiff, yDiff))
			monsterMoveCheckX(entityIndex, xDiff, yDiff);
	} else {
		if (!monsterMoveCheckX(entityIndex, xDiff, yDiff))
			monsterMoveCheckY(entityIndex, xDiff, yDiff);
	}
}

void OverworldLogic::monsterAttack(int entityIndex, int xDiff, int yDiff, int distance) {
	// WORKAROUND: the original animates the attack as a projectile flying
	// tile-by-tile towards the player, using up to distance tiles and
	// stopping early if it's blocked by mountains. That's purely visual and
	// doesn't affect whether the attack hits, so it's skipped here
	const auto &e = _G(savegame)._overworldEntities[entityIndex];
	int monsterIdx = (e._type - Data::TILE_FIRST_MONSTER) / 2;

	writeString(Data::OVERWORLD_MONSTERS[monsterIdx]);
	writeString(" attacks!");

	auto &sg = _G(savegame);
	int roll = getRandomNumber(1, 255);
	int threshold = 200 - sg._stamina / 2 - sg._equippedArmor * 8;

	if (roll >= threshold) {
		writeString("\n");
		writeString("Missed!\n");
		return;
	}

	int damage = getRandomNumber(1, Data::OVERWORLD_MONSTERS_DAMAGE[monsterIdx] * 2 + 1);
	playFX(2);

	writeString("\n");
	writeString("Hit! ");
	writeString("%d damage\n", damage);

	sg._hits -= damage;
	redrawStats();
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
