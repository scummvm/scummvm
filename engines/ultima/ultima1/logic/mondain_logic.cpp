
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

static int sgn(int v) {
	return (v > 0) - (v < 0);
}

MondainLogic::MondainLogic() {
	_G(map)._mapType = Data::MAPTYPE_MONDAIN;

	// The Mondain encounter's own cityCastleAttackDir uses a weaker per-
	// point-of-strength scale (*3) than the main game's (*8)
	_weaponDamageScale = 3;
}

void MondainLogic::entering() {
	Data::Savegame &sg = _G(savegame);

	// Discard anything a previous visit left in the room (placed barriers,
	// conjured hazards, stale position markers)
	_G(map).resetMondainMap();

	// Fixed starting positions - the player by the time machine on the west
	// wall, Mondain across the room to the east. These match the MTILE_PLAYER
	// marker baked into MONDAIN_MAP; Mondain is drawn purely as an overlay,
	// so his cell stays 0 in the map data
	sg._locationPosition = Common::Point(3, 4);
	sg._mondainPos = Common::Point(15, 4);

	// The original re-launches a dedicated executable for this encounter, so
	// every bit of its state starts from that executable's static
	// initializers. We reuse the one long-lived Savegame, so reset the same
	// state here to those values - otherwise a second visit (via the debug
	// map command, say) would inherit a dead or half-fought Mondain
	sg._mondainHits = 1000;
	sg._mondainPhase = 2;
	sg._mondainPhaseAnimOffset = 0;
	sg._mondainPhaseTimer = 20;
	sg._mondainHitAnimFrame = 0;
	sg._mondainHitAnimDir = 0;
	sg._mondainCombatFlag = 0;
	sg._gemDestroyedFlag = false;
	sg._mondainDefeatedFlag = 0;
	sg._mondainHazardAnim = -1;
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

	// Advance the conjured-hazard flash. It plays 6 frames, then the cell
	// it occupied is cleared back to open floor
	if (sg._mondainHazardAnim >= 0) {
		if (sg._mondainHazardAnim >= 6) {
			_G(map)[sg._mondainHazardPos.y][sg._mondainHazardPos.x] = 0;
			sg._mondainHazardAnim = -1;
		} else {
			++sg._mondainHazardAnim;
		}
	}

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

void MondainLogic::updateMondainState() {
	Data::Savegame &sg = _G(savegame);

	// Once combat's underway, Mondain occasionally conjures a hazard tile
	// onto a random empty cell that shares neither a row nor a column with
	// him or the player. It flashes for a few frames (see tick()) and burns
	// the player if they walk into it
	if (sg._mondainCombatFlag && sg._mondainHazardAnim < 0 && getRandomNumber(1, 200) < 36) {
		int hx = 0, hy = 0;
		bool empty = false;

		for (int attempt = 0; attempt < 100 && !empty; ++attempt) {
			hx = (getRandomNumber(1, 255) + getRandomNumber(1, 255)) % Data::MONDAIN_WIDTH;
			hy = (getRandomNumber(1, 255) + getRandomNumber(1, 255)) % Data::MONDAIN_HEIGHT;
			empty = _G(map).getMapTile(hx, hy) == 0;
		}

		if (empty && hx != sg._mondainPos.x && hx != sg._locationPosition.x &&
				hy != sg._mondainPos.y && hy != sg._locationPosition.y) {
			sg._mondainHazardAnim = 0;
			sg._mondainHazardPos = Common::Point(hx, hy);
			_G(map)[hy][hx] = Data::MTILE_HAZARD;
		}
	}

	// Step Mondain through his hit-point-driven phases
	if (sg._mondainHits > 500) {
		// Healthy - his aggressive phase
		sg._mondainPhase = 2;
		sg._mondainPhaseTimer = 20;
	} else if (sg._mondainHits > 0) {
		// Wounded - backs off and stops attacking
		if (sg._mondainPhase != 11) {
			sg._mondainPhase = 11;
			sg._mondainPhaseTimer = 40;
			sg._mondainPhaseAnimOffset = 0;
		}
	} else if (sg._mondainPhaseTimer != 30) {
		// Hit points have just reached zero
		writeString("Mondain is dead!");

		if (!sg._gemDestroyedFlag) {
			// The gem still stands, so this won't last - mondainTakeTurn
			// will heal him back up
			writeString("...or is he?\n");
		} else {
			writeString("\n");
			sg._mondainDefeatedFlag = 1;
		}

		sg._mondainPhase = 4;
		sg._mondainPhaseTimer = 30;
		sg._mondainPhaseAnimOffset = 0;
	}
}

void MondainLogic::updateCreatures() {
	mondainTakeTurn();
}

void MondainLogic::mondainTakeTurn() {
	Data::Savegame &sg = _G(savegame);

	if (!sg._mondainCombatFlag) {
		// Combat hasn't been triggered yet - just an occasional hint that
		// something's afoot
		if (getRandomNumber(1, 200) < 17)
			writeString("You hear a strange chanting!\n");
		return;
	}

	if (sg._mondainPhase == 4) {
		// Defeated for now, but the gem's intact - he steadily heals back
		sg._mondainHits += 25;
		return;
	}

	int dx = sg._mondainPos.x - sg._locationPosition.x;
	int dy = sg._mondainPos.y - sg._locationPosition.y;
	int dist = ABS(dx) + ABS(dy);

	if (dist == 1 && sg._mondainPhaseTimer == 20) {
		mondainMeleeAttack();
		return;
	}

	if (dist < 7 && getRandomNumber(1, 255) < 128 && sg._mondainPhaseTimer == 20) {
		mondainSpellAttack();
		return;
	}

	// Otherwise he moves. When retreating (his wounded phase) and already
	// lined up with the player on an axis, he picks that axis at random so
	// he doesn't just sit in the firing line
	if (sg._mondainPhaseTimer == 40) {
		if (dx == 0)
			dx = randomSign();
		if (dy == 0)
			dy = randomSign();
	}

	bool moved;
	if (getRandomNumber(1, 255) & 1) {
		moved = tryMoveMondain(0, sgn(dy));
		if (!moved)
			moved = tryMoveMondain(sgn(dx), 0);
	} else {
		moved = tryMoveMondain(sgn(dx), 0);
		if (!moved)
			moved = tryMoveMondain(0, sgn(dy));
	}

	// Boxed in while retreating - he burns a turn regenerating instead
	if (!moved && sg._mondainPhaseTimer == 40)
		sg._mondainHits += 5;
}

bool MondainLogic::tryMoveMondain(int dx, int dy) {
	Data::Savegame &sg = _G(savegame);
	int newX = sg._mondainPos.x + dx;
	int newY = sg._mondainPos.y + dy;

	if (newX < 0 || newY < 0 || newX >= Data::MONDAIN_WIDTH || newY >= Data::MONDAIN_HEIGHT)
		return false;

	// Anything non-empty blocks him - walls, barriers, a hazard, and the
	// player (whose cell is kept marked with MTILE_PLAYER)
	if (_G(map).getMapTile(newX, newY) != 0)
		return false;

	sg._mondainPos = Common::Point(newX, newY);
	return true;
}

int MondainLogic::randomSign() {
	return sgn(getRandomNumber(1, 255) - 128);
}

void MondainLogic::damagePlayer(int amount) {
	Data::Savegame &sg = _G(savegame);

	writeString("Hit! ");
	writeString("%d damage!\n", amount);

	sg._hits -= amount;
	playFX(2);
	redrawStats();
}

void MondainLogic::mondainMeleeAttack() {
	Data::Savegame &sg = _G(savegame);

	writeString("Attacked by Mondain!\n");
	playFX(3);

	// Note: as in the original, higher stamina/armour actually *raises* this
	// threshold, so a well-equipped player is hit more often, not less
	int roll = getRandomNumber(1, 255);
	int threshold = 2 * (0xB9 - sg._stamina / 2 + (sg._equippedArmor << 3));

	if (roll <= threshold)
		damagePlayer(sg._hits / 32 + getRandomNumber(1, 20));
	else
		writeString("Missed!\n");
}

void MondainLogic::mondainSpellAttack() {
	Data::Savegame &sg = _G(savegame);

	int kind = getRandomNumber(1, 238) % 3;
	writeString("Mondain casts ");

	bool hit = false;

	switch (kind) {
	case 0:
		// Magic missile - straightforward damage, dodged with high mental
		// stats (the threshold goes negative once wisdom is high enough)
		writeString("magic missile!\n");
		playFX(5);

		hit = getRandomNumber(1, 255) < 2 * (0xC3 - (sg._wisdom + sg._intelligence / 2));
		if (hit)
			damagePlayer(getRandomNumber(1, 100));
		break;

	case 1:
		// Mind blaster - drains each of the six attributes
		writeString("mind blaster!\n");
		playFX(5);

		hit = getRandomNumber(1, 200) > 140;
		if (hit) {
			writeString("Hit!  Stats are reduced!\n");

			int16 *stats[6] = {
				&sg._strength, &sg._agility, &sg._stamina,
				&sg._charisma, &sg._wisdom, &sg._intelligence
			};
			for (int i = 0; i < 6; ++i) {
				if (*stats[i] > 0x10)
					*stats[i] -= *stats[i] / 8;
				else if (*stats[i] >= 2)
					*stats[i] -= 2;
			}
		}
		break;

	default:
		// Psionic shock
		writeString("psionic shock!\n");
		playFX(5);

		hit = getRandomNumber(1, 200) > 140;
		if (hit) {
			int amount = sg._hits / 32;
			if (amount > 0xFF)
				amount -= 0xFE;
			damagePlayer(amount);
		}
		break;
	}

	if (!hit)
		writeString("Missed!\n");
}

void MondainLogic::endOfTurn() {
	// Not chaining to Logic::endOfTurn() on purpose

	updateMondainState();

	if (_G(savegame)._mondainDefeatedFlag) {
		redrawStats();
		g_engine->addView("Win");
		return;
	}

	updateCreatures();

	redrawStats();

	if (_G(savegame)._hits <= 0)
		g_engine->addView("Lose");
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
