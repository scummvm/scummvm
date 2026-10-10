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

#include "ultima/ultima3/logic/dungeon_logic.h"
#include "ultima/ultima3/logic/location_logic.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {

constexpr int LAST_LEVEL = Data::DUNGEON_LEVELS - 1;
constexpr byte TILE_BLOCKED = 0x80;
constexpr byte TILE_NO_TURNING = 0xA0;
constexpr byte FIRST_MONSTER = 0x18;
constexpr int LAST_MONSTER_KIND = 6;

// How far a step in each direction goes
static const int FACING_DX[4] = { 0, 1, 0, -1 };
static const int FACING_DY[4] = { -1, 0, 1, 0 };

static int rollBelow(int limit) {
	return Graphics::Views::g_events->getRandomNumber(limit - 1);
}

byte DungeonLogic::tile() const {
	const Data::Savegame &save = _G(savegame);
	return _G(dungeon).tile(save._dungeonLevel, save._posX, save._posY);
}

bool DungeonLogic::moveForward() {
	Data::Savegame &save = _G(savegame);
	int x = (save._posX + FACING_DX[save._facing]) & (Data::DUNGEON_SIZE - 1);
	int y = (save._posY + FACING_DY[save._facing]) & (Data::DUNGEON_SIZE - 1);

	// Only the plainest walls can't be walked through
	if (!_G(intangible) && _G(dungeon).tile(save._dungeonLevel, x, y) == TILE_BLOCKED)
		return false;

	save._posX = x;
	save._posY = y;
	return true;
}

bool DungeonLogic::moveBackward() {
	Data::Savegame &save = _G(savegame);
	int behind = (save._facing + 2) & 3;
	int x = (save._posX + FACING_DX[behind]) & (Data::DUNGEON_SIZE - 1);
	int y = (save._posY + FACING_DY[behind]) & (Data::DUNGEON_SIZE - 1);

	if (!_G(intangible) && (_G(dungeon).tile(save._dungeonLevel, x, y) & TILE_BLOCKED))
		return false;

	save._posX = x;
	save._posY = y;
	return true;
}

bool DungeonLogic::turn(bool right) {
	Data::Savegame &save = _G(savegame);
	if (tile() >= TILE_NO_TURNING)
		return false;

	save._facing = (save._facing + (right ? 1 : 3)) & 3;
	return true;
}

DungeonLogic::Ladder DungeonLogic::climb() {
	Data::Savegame &save = _G(savegame);
	if (!(tile() & Data::DTILE_LADDER_UP))
		return LADDER_NONE;

	if (save._dungeonLevel == 0) {
		LocationLogic().exitToWorld();
		return LADDER_OUT;
	}

	--save._dungeonLevel;
	return LADDER_TAKEN;
}

bool DungeonLogic::descend() {
	Data::Savegame &save = _G(savegame);
	byte here = tile();

	if ((here & TILE_BLOCKED) || !(here & Data::DTILE_LADDER_DOWN) || save._dungeonLevel >= LAST_LEVEL)
		return false;

	++save._dungeonLevel;
	return true;
}

int DungeonLogic::rollEncounter() {
	Data::Savegame &save = _G(savegame);
	if (tile() != 0)
		return -1;

	// The deeper the party goes the likelier and fiercer the monsters
	if (rollBelow(0x82 + save._dungeonLevel) < 0x80)
		return -1;

	int kind = MIN(rollBelow(save._dungeonLevel + 2), LAST_MONSTER_KIND);

	// Where they were met is left as a chest
	_G(dungeon).setTile(save._dungeonLevel, save._posX, save._posY, Data::DTILE_CHEST);
	return FIRST_MONSTER + kind;
}

void DungeonLogic::teleportRandomly() {
	Data::Savegame &save = _G(savegame);

	for (;;) {
		int x = rollBelow(Data::DUNGEON_SIZE), y = rollBelow(Data::DUNGEON_SIZE);

		if (_G(dungeon).tile(save._dungeonLevel, x, y) == 0) {
			save._posX = x;
			save._posY = y;
			return;
		}
	}
}

} // namespace Ultima3
} // namespace Ultima
