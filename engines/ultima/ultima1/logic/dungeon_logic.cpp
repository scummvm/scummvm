
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

#include "ultima/ultima1/logic/dungeon_logic.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

DungeonLogic::DungeonLogic() {
	_G(map)._mapType = Data::MAPTYPE_DUNGEON;
}

void DungeonLogic::entering() {
	_G(savegame)._locationPosition = Common::Point(1, 1);
	_G(savegame)._dungeonLevel = 1;
	_G(savegame)._dungeonDirection = 4;
	_G(savegame)._dungeonExitHitPoints = 0;
}

int DungeonLogic::getDirDeltaX() const {
	switch (_G(savegame)._dungeonDirection) {
	case Data::DIR_RIGHT:
		return 1;
	case Data::DIR_LEFT:
		return -1;
	default:
		return 0;
	}
}

int DungeonLogic::getDirDeltaY() const {
	switch (_G(savegame)._dungeonDirection) {
	case Data::DIR_UP:
		return -1;
	case Data::DIR_DOWN:
		return 1;
	default:
		return 0;
	}
}

bool DungeonLogic::isBlocked(int deltaX, int deltaY) const {
	const auto &pos = _G(savegame)._locationPosition;
	const Data::DungeonCell &destCell = _G(dungeon)._cells[pos.y + deltaY][pos.x + deltaX];

	if (destCell._tileNum == Data::DTILE_WALL || destCell._tileNum == Data::DTILE_SECRET_DOOR ||
			destCell._tileNum == Data::DTILE_BEAMS)
		return true;

	if (destCell._tileNum == Data::DTILE_DOOR) {
		// A door only blocks if the player is already standing on one -
		// otherwise stepping through it is allowed
		if (_G(dungeon)._cells[pos.y][pos.x]._tileNum == Data::DTILE_DOOR)
			return true;
	}

	return destCell._monsterId != Data::DUNGEON_NO_MONSTER;
}

void DungeonLogic::turnLeft() {
	writeString("Turn left\n");

	switch (_G(savegame)._dungeonDirection) {
	case Data::DIR_LEFT:
		_G(savegame)._dungeonDirection = Data::DIR_DOWN;
		break;
	case Data::DIR_RIGHT:
		_G(savegame)._dungeonDirection = Data::DIR_UP;
		break;
	case Data::DIR_UP:
		_G(savegame)._dungeonDirection = Data::DIR_LEFT;
		break;
	case Data::DIR_DOWN:
		_G(savegame)._dungeonDirection = Data::DIR_RIGHT;
		break;
	}
}

void DungeonLogic::turnRight() {
	writeString("Turn right\n");

	switch (_G(savegame)._dungeonDirection) {
	case Data::DIR_LEFT:
		_G(savegame)._dungeonDirection = Data::DIR_UP;
		break;
	case Data::DIR_RIGHT:
		_G(savegame)._dungeonDirection = Data::DIR_DOWN;
		break;
	case Data::DIR_UP:
		_G(savegame)._dungeonDirection = Data::DIR_RIGHT;
		break;
	case Data::DIR_DOWN:
		_G(savegame)._dungeonDirection = Data::DIR_LEFT;
		break;
	}
}

void DungeonLogic::turnAround() {
	writeString("Turn around\n");

	switch (_G(savegame)._dungeonDirection) {
	case Data::DIR_LEFT:
		_G(savegame)._dungeonDirection = Data::DIR_RIGHT;
		break;
	case Data::DIR_RIGHT:
		_G(savegame)._dungeonDirection = Data::DIR_LEFT;
		break;
	case Data::DIR_UP:
		_G(savegame)._dungeonDirection = Data::DIR_DOWN;
		break;
	case Data::DIR_DOWN:
		_G(savegame)._dungeonDirection = Data::DIR_UP;
		break;
	}
}

void DungeonLogic::moveForward() {
	int deltaX = getDirDeltaX();
	int deltaY = getDirDeltaY();

	writeString("Forward");

	if (isBlocked(deltaX, deltaY)) {
		playFX(0);
		writeString(" - path blocked!\n");
	} else {
		_G(savegame)._locationPosition.x += deltaX;
		_G(savegame)._locationPosition.y += deltaY;
		writeString("\n");
	}
}

void DungeonLogic::climbLadderUp() {
	Data::Savegame &sg = _G(savegame);
	--sg._dungeonLevel;
	writeString(" up to level %d\n", sg._dungeonLevel);

	if (sg._dungeonLevel == 0) {
		// Exiting the dungeon back to the overworld
		int hp = sg._hits + sg._dungeonExitHitPoints;
		if (hp > 9999)
			sg._dungeonExitHitPoints = 9999 - sg._hits;

		if (sg._dungeonExitHitPoints != 0) {
			writeString("Thou dost gain %d hit ", sg._dungeonExitHitPoints);
			if (sg._dungeonExitHitPoints > 999)
				writeString("\n");
			writeString("points\n");

			sg._hits += sg._dungeonExitHitPoints;
			redrawStats();
		}

		_G(map).load(Data::MAP_OVERWORLD);
	} else {
		_G(dungeon).generateDungeonLevel();
		_G(dungeon)._cells[sg._locationPosition.y][sg._locationPosition.x]._monsterId = Data::DUNGEON_NO_MONSTER;
	}
}

void DungeonLogic::climbLadderDown() {
	Data::Savegame &sg = _G(savegame);
	++sg._dungeonLevel;
	writeString(" down to level %d\n", sg._dungeonLevel);

	_G(dungeon).generateDungeonLevel();
}

bool DungeonLogic::move(Data::Direction dir) {
	switch (dir) {
	case Data::DIR_LEFT:
		turnLeft();
		break;
	case Data::DIR_RIGHT:
		turnRight();
		break;
	case Data::DIR_UP:
		moveForward();
		break;
	case Data::DIR_DOWN:
		turnAround();
		break;
	}

	return true;
}

bool DungeonLogic::climb() {
	writeString("K-Limb");

	const Common::Point &pos = _G(savegame)._locationPosition;
	Data::DungeonTileId tileNum = _G(dungeon)._cells[pos.y][pos.x]._tileNum;

	if (tileNum != Data::DTILE_LADDER_UP && tileNum != Data::DTILE_LADDER_DOWN) {
		writeString(" what?\n");
		playFX(1);
		return true;
	}

	if (_G(savegame)._dungeonDirection == Data::DIR_LEFT ||
			_G(savegame)._dungeonDirection == Data::DIR_RIGHT) {
		writeString("\n");
		writeString("Thou must face the ladder!\n");
		playFX(1);
		return true;
	}

	if (tileNum == Data::DTILE_LADDER_UP)
		climbLadderUp();
	else
		climbLadderDown();

	return true;
}

void DungeonLogic::updateCreatures() {
	// TODO
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
