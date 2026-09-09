
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

#include "ultima/ultima1/logic/mondain_logic.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

static const int8 DELTA_X[5] = { 0, -1, 1, 0, 0 };
static const int8 DELTA_Y[5] = { 0, 0, 0, -1, 1 };

MondainLogic::MondainLogic() {
	_G(map)._mapType = Data::MAPTYPE_MONDAIN;
}

void MondainLogic::entering() {
	_G(savegame)._locationPosition = Common::Point(3, 4);
	_G(savegame)._mondainPos = Common::Point(15, 4);
}

bool MondainLogic::move(Data::Direction dir) {
	Data::Savegame &sg = _G(savegame);
	int newX = sg._locationPosition.x + DELTA_X[dir];
	int newY = sg._locationPosition.y + DELTA_Y[dir];

	int moveResult;
	if (newX < 0 || newX >= Data::MONDAIN_WIDTH || newY < 0 || newY >= Data::MONDAIN_HEIGHT) {
		// Off the edge of the room
		moveResult = 2;
	} else if (newX == sg._mondainPos.x && newY == sg._mondainPos.y) {
		// Mondain himself is blocking the way
		moveResult = 2;
	} else {
		moveResult = _G(map).getMapTile(newX, newY);
	}

	if (moveResult == 0) {
		// Open floor - move there. The old/new position markers embedded
		// in the map are kept in sync with the player's actual position,
		// even though MondainMap::draw() redraws the player as its own
		// overlay each frame regardless of what's in the map data
		writeString("%s\n", Data::DIRECTION_NAMES[dir]);

		_G(map)[sg._locationPosition.y][sg._locationPosition.x] = 0;
		sg._locationPosition = Common::Point(newX, newY);
		_G(map)[newY][newX] = Data::MTILE_PLAYER;
		playFX(4);

	} else if (moveResult == 1) {
		// The time machine - can't be reached this way
		writeString("Blocked!\n");
		playFX(0);

	} else {
		// Bumped into a wall, a gem, a bat, Mondain himself, or the edge of
		// the room - all of them burn the player a little
		playFX(0);

		int damage = sg._hits / 10;
		if (damage == 0)
			damage = 1;

		writeString("Burned! ");
		writeString("%d damage!\n", damage);
		playFX(2);

		sg._hits -= damage;
		redrawStats();
	}

	if (isAdjacentToMondain() && sg._mondainHitAnimFrame == 0) {
		sg._mondainHitAnimFrame = 1;
		sg._mondainHitAnimDir = 1;
		sg._mondainCombatFlag = 1;
	}

	return true;
}

void MondainLogic::tick() {
	// Only advance the animations once every 4 frames
	if (++_tickCounter < 4)
		return;
	_tickCounter = 0;

	Data::Savegame &sg = _G(savegame);

	// Bounce the hit-reaction frame back and forth between 1 and 4
	if (sg._mondainHitAnimFrame != 0) {
		sg._mondainHitAnimFrame += sg._mondainHitAnimDir;

		if (sg._mondainHitAnimFrame == 1)
			sg._mondainHitAnimDir = 1;
		else if (sg._mondainHitAnimFrame > 4)
			sg._mondainHitAnimDir = -1;
	}

	// Cycle the idle pose animation - what it does depends on which of
	// Mondain's story-driven phases is currently active
	if (sg._mondainPhase == 2) {
		sg._mondainPhaseAnimOffset = (getRandomNumber(1, 200) < 100) ? 1 : 0;
	} else if (sg._mondainPhase == 11) {
		if (sg._mondainPhaseAnimOffset == 2)
			sg._mondainPhaseAnimOffset = 0;
		else
			++sg._mondainPhaseAnimOffset;
	}
}

bool MondainLogic::isAdjacentToMondain() const {
	const Common::Point &pos = _G(savegame)._locationPosition;
	const Common::Point &mondainPos = _G(savegame)._mondainPos;

	int dx = ABS(pos.x - mondainPos.x);
	int dy = ABS(pos.y - mondainPos.y);
	return dx <= 1 && dy <= 1 && (dx != 0 || dy != 0);
}

void MondainLogic::updateCreatures() {
	// TODO
}

bool MondainLogic::get() {
	Data::Savegame &sg = _G(savegame);
	writeString("Get (Gem)");

	if (sg._gemDestroyedFlag || !isAdjacentToMondain()) {
		writeString("..'tis nothing here!\n");
		playFX(1);
		return true;
	}

	// Destroy the gem - it can only be done once, adjacent to Mondain
	sg._mondainCombatFlag = 1;
	sg._gemDestroyedFlag = true;
	sg._mondainHitAnimFrame = 0;

	int damage = sg._hits * 3 / 4;

	writeString("... ");
	writeString("%d damage!\n", damage);

	for (int i = 0; i < 5; ++i)
		playFX(2);

	writeString("The Gem is DESTROYED!\n");

	sg._hits -= damage;
	redrawStats();

	if (sg._mondainPhaseTimer == 30)
		sg._mondainDefeatedFlag = 1;

	return true;
}

bool MondainLogic::inform() {
	if (_G(savegame)._gemDestroyedFlag) {
		writeString("Inform and Search\n");
		writeString("Mondain's magical aura doth\n");
		writeString("seem substantially diminished\n");
		writeString("in the absence of the gem.\n");
	} else {
		writeString("Inform and Search...it looks\n");
		writeString("as if he is creating the evil\n");
		writeString("gem!\n");
	}

	return true;
}

bool MondainLogic::quit() {
	writeString("Quit...is not allowed!\n");
	playFX(1);
	return true;
}

bool MondainLogic::steal() {
	writeString("Steal...he's watching,\n");
	writeString("That would be foolish!\n");
	playFX(1);
	return true;
}

bool MondainLogic::transact() {
	writeString("Transact...Mondain will\n");
	writeString("not negotiate!\n");
	playFX(1);
	return true;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
