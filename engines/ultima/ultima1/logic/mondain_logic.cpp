
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

	// The Mondain encounter's own cityCastleAttackDir uses a weaker per-
	// point-of-strength scale (*3) than the main game's (*8)
	_weaponDamageScale = 3;
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

void MondainLogic::damage(Data::Direction dir, int effectNum, int maxDistance, int strike, int hitChance, int tileId) {
	playFX(effectNum);

	Data::Savegame &sg = _G(savegame);
	int dx = DELTA_X[dir], dy = DELTA_Y[dir];
	const Common::Point &pos = sg._locationPosition;

	int scanX = pos.x, scanY = pos.y;
	bool foundMondain = false;

	for (int step = 1; step <= maxDistance; ++step) {
		scanX = pos.x + dx * step;
		scanY = pos.y + dy * step;

		int tileVal = _G(map).getMapTile(scanX, scanY);
		foundMondain = tileVal == 0 && scanX == sg._mondainPos.x && scanY == sg._mondainPos.y;

		if (foundMondain || scanX <= 0 || scanY <= 0 ||
				scanX >= Data::MONDAIN_WIDTH || scanY >= Data::MONDAIN_HEIGHT)
			break;
	}

	if (!foundMondain || getRandomNumber(1, 100) > hitChance) {
		writeString("Missed!\n");
		return;
	}

	sg._mondainHits -= strike;
	writeString("Hit Mondain! ");
	writeString("%d damage!\n", strike);
	playFX(2);
}

bool MondainLogic::isWithinRange7() const {
	const Common::Point &pos = _G(savegame)._locationPosition;
	const Common::Point &mondainPos = _G(savegame)._mondainPos;
	return ABS(pos.x - mondainPos.x) + ABS(pos.y - mondainPos.y) < 7;
}

bool MondainLogic::rollSpellSuccess() {
	Data::Savegame &sg = _G(savegame);
	return getRandomNumber(1, 249) < (sg._intelligence + 160) || sg._class == Data::CLASS_WIZARD;
}

int MondainLogic::getMagicWeaponPower() {
	Data::Savegame &sg = _G(savegame);
	int result = getRandomNumber(2, sg._intelligence);

	if (sg._equippedWeapon == Data::WEAPON_WAND)
		result *= 2;
	else if (sg._equippedWeapon == Data::WEAPON_AMULET)
		result = result * 3 / 2;
	else if (sg._equippedWeapon == Data::WEAPON_STAFF || sg._equippedWeapon == Data::WEAPON_TRIANGLE)
		result *= 3;

	return result;
}

bool MondainLogic::castTeleport() {
	writeString("\n");

	if (!rollSpellSuccess())
		return false;

	Data::Savegame &sg = _G(savegame);
	int newX = 0, newY = 0;
	bool valid = false;

	// The original retries with no attempt cap at all - bounded here for
	// safety, matching the same defensive pattern used for the dungeon and
	// overworld Blink spells
	for (int attempt = 0; attempt < 500 && !valid; ++attempt) {
		newX = getRandomNumber(1, 255) % Data::MONDAIN_WIDTH;
		newY = getRandomNumber(1, 255) % Data::MONDAIN_HEIGHT;
		valid = _G(map).getMapTile(newX, newY) == 0;
	}

	if (!valid)
		return false;

	_G(map)[sg._locationPosition.y][sg._locationPosition.x] = 0;
	sg._locationPosition = Common::Point(newX, newY);
	_G(map)[newY][newX] = Data::MTILE_PLAYER;

	playFX(5);
	writeString("Done.\n");
	return true;
}

bool MondainLogic::castInterficioNunc() {
	writeString("\n");

	if (!rollSpellSuccess() || !isWithinRange7())
		return false;

	// The Kill spell doesn't work on Mondain at all - it backfires,
	// doubling his remaining hit points instead (capped at 15000)
	writeString("\"INTERFICIO-NUNC!\"\n");
	writeString("The spell doth seem\n");
	writeString("to make him stronger!\n");

	Data::Savegame &sg = _G(savegame);
	if (sg._mondainHits < 15000)
		sg._mondainHits *= 2;

	playFX(5);
	return true;
}

bool MondainLogic::castPlaceBarrier(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("nothing\n");
		playFX(6);
		return true;
	}

	writeString("%s\n", Data::DIRECTION_NAMES[dir]);
	if (!rollSpellSuccess())
		return false;

	Data::Savegame &sg = _G(savegame);
	int x = sg._locationPosition.x + DELTA_X[dir];
	int y = sg._locationPosition.y + DELTA_Y[dir];

	// Can't place a barrier on an occupied tile, or anywhere in line with
	// Mondain
	if (_G(map).getMapTile(x, y) != 0 || x == sg._mondainPos.x || y == sg._mondainPos.y)
		return false;

	_G(map)[y][x] = 5;
	playFX(5);
	writeString("Done.\n");
	return true;
}

bool MondainLogic::castRemoveBarrier(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("nothing\n");
		playFX(6);
		return true;
	}

	writeString("%s\n", Data::DIRECTION_NAMES[dir]);
	if (!rollSpellSuccess())
		return false;

	Data::Savegame &sg = _G(savegame);
	int x = sg._locationPosition.x + DELTA_X[dir];
	int y = sg._locationPosition.y + DELTA_Y[dir];

	if (_G(map).getMapTile(x, y) != 5)
		return false;

	_G(map)[y][x] = 0;
	playFX(5);
	writeString("Done.\n");
	return true;
}

bool MondainLogic::castMagicMissile(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("nothing\n");
		playFX(6);
		return false;
	}

	writeString("%s\n", Data::DIRECTION_NAMES[dir]);
	if (!rollSpellSuccess() || !isWithinRange7())
		return false;

	int strike = getMagicWeaponPower();
	damage(dir, 5, 5, strike, 500, 0);
	return true;
}

bool MondainLogic::cast() {
	Data::Savegame &sg = _G(savegame);
	int spell = sg._equippedSpell;

	writeString("Cast %s", Data::SPELL_NAMES[spell]);

	if (sg._spells[spell] == 0) {
		writeString("\n");
		writeString("You've used up that spell!\n");
		playFX(6);
		return true;
	}

	if (spell != Data::SPELL_PRAYER)
		--sg._spells[spell];

	if (spell != Data::SPELL_MAGIC_MISSILE && spell != Data::SPELL_BLINK && spell != Data::SPELL_CREATE &&
			spell != Data::SPELL_DESTROY && spell != Data::SPELL_KILL) {
		// Nothing else has any effect on Mondain
		writeString("\n");
		writeString("Failed!\n");
		playFX(6);
		return true;
	}

	bool success;
	switch (spell) {
	case Data::SPELL_BLINK:
		success = castTeleport();
		break;
	case Data::SPELL_KILL:
		success = castInterficioNunc();
		break;
	default:
		// Magic Missile/Create/Destroy all need a direction first, so
		// defer to the Direction view - castSpellAttack() picks up from
		// here once one's been chosen
		writeString(": ");
		_castSpell = spell;
		_G(logic) = Common::SharedPtr<Logic>(new DirectionLogic(DirectionLogic::SPELL));
		g_engine->addView("Direction");
		return false;
	}

	if (!success) {
		writeString("Failed!\n");
		playFX(6);
	}
	return true;
}

void MondainLogic::castSpellAttack(Data::Direction dir) {
	bool success;
	switch (_castSpell) {
	case Data::SPELL_CREATE:
		success = castPlaceBarrier(dir);
		break;
	case Data::SPELL_DESTROY:
		success = castRemoveBarrier(dir);
		break;
	default:
		// SPELL_MAGIC_MISSILE
		success = castMagicMissile(dir);
		break;
	}

	if (!success) {
		writeString("Failed!\n");
		playFX(6);
	}
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
