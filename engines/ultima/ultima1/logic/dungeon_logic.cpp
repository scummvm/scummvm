
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
	if (_G(savegame)._dungeonLevel == 0)
		_G(savegame)._dungeonLevel = 1;
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

bool DungeonLogic::attack(Data::Direction dir) {
	writeString("Attack with %s", Data::WEAPON_NAMES_LOWER[_G(savegame)._equippedWeapon]);

	int maxDistance = Data::WEAPONS_DISTANCE[_G(savegame)._equippedWeapon];
	if (!maxDistance) {
		// It's a non-attacking "weapon" like the rope
		writeString("?\n");
		playFX(1);

	} else {
		writeString("\n");

		int monsterDistance = getMonsterDistance(maxDistance);
		if (!monsterDistance) {
			writeString("Nothing\n");

		} else {
			int agility = _G(savegame)._agility + 50;
			int strike = getRandomNumber(2, _G(savegame)._equippedWeapon * 8 + _G(savegame)._strength);
			int deltaX = getDirDeltaX();
			int deltaY = getDirDeltaY();
			int x = _G(savegame)._locationPosition.x + monsterDistance * deltaX;
			int y = _G(savegame)._locationPosition.y + monsterDistance * deltaY;

			dungeonAttackAt(2, agility, strike, x, y);
		}
	}

	return true;
}

bool DungeonLogic::cast() {
	writeString("Cast %s", Data::SPELL_NAMES[_G(savegame)._equippedSpell]);

	int spell = _G(savegame)._equippedSpell;
	if (spell == Data::SPELL_PRAYER) {
		castPrayer();

	} else if (!_G(savegame)._spells[spell]) {
		writeString("\n");
		playFX(6);
		writeString("You've used up that spell!\n");

	} else {
		castSpell(spell, true);
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

void DungeonLogic::showNearbyText() {
	const Common::Point &pos = _G(savegame)._locationPosition;

	// A chest/coffin at the player's own feet is only noticed when they're
	// standing in a doorway, matching the original's dungeonUpdate
	const Data::DungeonCell &hereCell = _G(dungeon)._cells[pos.y][pos.x];
	if (hereCell._tileNum == Data::DTILE_DOOR) {
		if (hereCell._itemId == Data::DITEM_COFFIN)
			writeString("Coffin\n");
		else if (hereCell._itemId == Data::DITEM_CHEST)
			writeString("Chest\n");
	}

	// Items/monsters visible ahead, in the direction currently faced, up to
	// wherever the dungeon view itself would be blocked (capped at 5 tiles,
	// matching the view's own render distance)
	int deltaX = getDirDeltaX();
	int deltaY = getDirDeltaY();
	int x = pos.x, y = pos.y;

	for (int distance = 1; distance <= 5; ++distance) {
		x += deltaX;
		y += deltaY;
		const Data::DungeonCell &cell = _G(dungeon)._cells[y][x];

		if (cell._itemId == Data::DITEM_COFFIN)
			writeString("Coffin\n");
		else if (cell._itemId == Data::DITEM_CHEST)
			writeString("Chest\n");

		bool hasMonster = cell._monsterId != Data::DUNGEON_NO_MONSTER;
		if (hasMonster) {
			writeString(Data::UNDERWORLD_MONSTERS[cell._monsterId]);
			writeString("\n");
		}

		// Disguised monster types (mimic, invisible seeker, gelatinous
		// cube) don't block the view/movement, so scanning continues past
		// them - matching DungeonMap::isMonsterBlocking
		bool isBlockingMonster = hasMonster && cell._monsterId != Data::UMONS_GELATINOUS_CUBE &&
			cell._monsterId != Data::UMONS_MIMIC && cell._monsterId != Data::UMONS_INVISIBLE_SEEKER;

		if (cell._tileNum == Data::DTILE_WALL || cell._tileNum == Data::DTILE_SECRET_DOOR ||
				cell._tileNum == Data::DTILE_DOOR || isBlockingMonster)
			break;
	}
}

void DungeonLogic::endOfTurn() {
	showNearbyText();

	// Calls updateCreatures() in turn
	Logic::endOfTurn();
}

int DungeonLogic::getMonsterDistance(int maxDistance) const {
	int deltaX = getDirDeltaX();
	int deltaY = getDirDeltaY();
	int x, y, tile = 0, monsterId = -1;
	int dist;

	for (dist = 0; dist < maxDistance; ++dist) {
		x = _G(savegame)._locationPosition.x + deltaX * dist;
		y = _G(savegame)._locationPosition.y + deltaY * dist;
		tile = _G(dungeon)._cells[y][x]._tileNum;
		monsterId = _G(dungeon)._cells[y][x]._monsterId;

		if (monsterId != -1 || tile == Data::DTILE_WALL || tile == Data::DTILE_SECRET_DOOR ||
			tile == Data::DTILE_BEAMS || tile == Data::DTILE_DOOR)
			break;
	}

	return (monsterId != -1 && tile != Data::DTILE_WALL &&
		tile != Data::DTILE_SECRET_DOOR && tile != Data::DTILE_BEAMS) ? dist : 0;;
}

void DungeonLogic::writeMonsterName(int monsterId) {
	if (monsterId == Data::UMONS_MIMIC)
		writeString("Mimic");
	else
		writeString(Data::UNDERWORLD_MONSTERS[monsterId]);
}

void DungeonLogic::monsterDead(int monsterId) {
	// Only 4 monster Ids have a castle quest tied to them - the ones named
	// in King::QUEST_MONSTER_NAMES, indexed by continent
	int castleIdx;
	switch (monsterId) {
	case 9:
		castleIdx = 2;
		break;
	case 14:
		castleIdx = 4;
		break;
	case 19:
		castleIdx = 6;
		break;
	case 24:
		castleIdx = 8;
		break;
	default:
		return;
	}

	Data::Savegame &sg = _G(savegame);
	if (sg._questStatus[castleIdx] == -1) {
		sg._questStatus[castleIdx] = 1;
		writeString("A quest has been completed!\n");
		playFX(5);
		writeString("\n");
	}
}

void DungeonLogic::giveCoins(int coins) {
	Data::Savegame &sg = _G(savegame);
	if (sg._coins + coins > 9999)
		coins = 9999 - sg._coins;

	sg._coins += coins;
	writeString("%d gold\n", coins);
}

void DungeonLogic::dungeonAttackAt(int effectNum, int agility, int strike, int x, int y) {
	Data::MapDungeon &dungeon = _G(dungeon);
	Data::Savegame &sg = _G(savegame);
	const Common::Point &pos = sg._locationPosition;

	// If the player is currently standing in a doorway, the shot can only
	// connect if it's heading straight down a clear corridor - not through
	// another door, or into a chest/coffin
	bool canHit = true;
	if (dungeon._cells[pos.y][pos.x]._tileNum == Data::DTILE_DOOR) {
		Data::DungeonTileId targetTile = dungeon._cells[y][x]._tileNum;
		canHit = targetTile == Data::DTILE_HALLWAY || targetTile == Data::DTILE_LADDER_UP ||
			targetTile == Data::DTILE_LADDER_DOWN;
	}

	if (getRandomNumber(1, 100) > agility) {
		writeString("Missed!\n");
		return;
	}

	Data::DungeonTileId hereTile = dungeon._cells[pos.y][pos.x]._tileNum;
	if (hereTile == Data::DTILE_WALL || hereTile == Data::DTILE_SECRET_DOOR ||
			hereTile == Data::DTILE_BEAMS || !canHit) {
		writeString("Missed!\n");
		return;
	}

	playFX(effectNum);
	if (strike != 10000)
		writeString("Hit! ");

	Data::DungeonCell &cell = dungeon._cells[y][x];
	if (cell._monsterHp <= strike) {
		// Monster killed
		writeMonsterName(cell._monsterId);
		writeString(strike == 10000 ? " destroyed!\n" : " killed!\n");

		monsterDead(cell._monsterId);

		int coins = getRandomNumber(2, sg._dungeonLevel * 3 + 10 + cell._monsterId);
		writeString("Thou dost find: ");
		giveCoins(coins);

		dungeon.killMonster(x, y);

		int expGain = getRandomNumber(2, sg._dungeonLevel * sg._dungeonLevel + 10);
		sg._experience += expGain;
		sg._dungeonExitHitPoints += expGain * 2;
		redrawStats();
	} else {
		// Damaged, but survived
		writeString("%d damage!\n", strike);
		cell._monsterHp -= strike;
	}
}


void DungeonLogic::castPrayer() {
	writeString("\n%s\n", Data::SPELL_PHRASES[0]);

	int spell = getRandomNumber(Data::SPELL_OPEN, Data::SPELL_KILL);
	if (spell == Data::SPELL_STEAL)
		spell = Data::SPELL_LADDER_DOWN;

	castSpell(spell, false);
}

int DungeonLogic::getMagicWeaponPower() {
	Data::Savegame &sg = _G(savegame);
	int result = getRandomNumber(1, sg._intelligence);

	if (sg._equippedWeapon == Data::WEAPON_WAND)
		result *= 2;
	else if (sg._equippedWeapon == Data::WEAPON_AMULET)
		result = result * 3 / 2;
	else if (sg._equippedWeapon == Data::WEAPON_STAFF || sg._equippedWeapon == Data::WEAPON_TRIANGLE)
		result *= 3;

	return result;
}

void DungeonLogic::castOpen() {
	Data::Savegame &sg = _G(savegame);
	writeString(" opened!\n");

	_G(dungeon)._cells[sg._locationPosition.y][sg._locationPosition.x]._itemId = Data::DITEM_NONE;

	writeString("Thou dost find: ");
	int coins = getRandomNumber(3, sg._dungeonLevel * sg._dungeonLevel + 9);
	giveCoins(coins);
}

void DungeonLogic::castSpell(int spell, bool showPhrase) {
	if (showPhrase)
		writeString("%s\n", Data::SPELL_PHRASES[spell]);

	Data::MapDungeon &dungeon = _G(dungeon);
	Data::Savegame &sg = _G(savegame);
	const Common::Point &pos = sg._locationPosition;
	bool failed = false;

	switch (spell) {
	case Data::SPELL_OPEN: {
		Data::DungeonCell &cell = dungeon._cells[pos.y][pos.x];
		if (cell._itemId == Data::DITEM_COFFIN) {
			writeString("Coffin");
			castOpen();
		} else {
			failed = true;
		}
		break;
	}

	case Data::SPELL_UNLOCK: {
		Data::DungeonCell &cell = dungeon._cells[pos.y][pos.x];
		if (cell._itemId == Data::DITEM_CHEST) {
			writeString("Chest");
			castOpen();
		} else {
			failed = true;
		}
		break;
	}

	case Data::SPELL_MAGIC_MISSILE: {
		int distance = getMonsterDistance(5);
		if (!distance) {
			failed = true;
		} else {
			int strike = getMagicWeaponPower();
			int x = pos.x + getDirDeltaX() * distance;
			int y = pos.y + getDirDeltaY() * distance;
			dungeonAttackAt(5, 101, strike, x, y);
		}
		break;
	}

	case Data::SPELL_STEAL:
		// Has no dungeon effect - same as the original's jump table, which
		// points this spell straight at its no-op default case
		break;

	case Data::SPELL_LADDER_DOWN:
		if (sg._dungeonLevel == 10 || dungeon._cells[pos.y][pos.x]._tileNum == Data::DTILE_BEAMS) {
			failed = true;
		} else if ((pos.x & 1) || (pos.y & 1)) {
			dungeon._cells[pos.y][pos.x]._tileNum = Data::DTILE_LADDER_DOWN;
			writeString("Ladder created!\n");
		} else {
			failed = true;
		}
		break;

	case Data::SPELL_LADDER_UP:
		if (!((pos.x & 1) || (pos.y & 1))) {
			failed = true;
		} else if (dungeon._cells[pos.y][pos.x]._tileNum == Data::DTILE_BEAMS) {
			failed = true;
		} else {
			dungeon._cells[pos.y][pos.x]._tileNum = Data::DTILE_LADDER_UP;
			writeString("Ladder created!\n");
		}
		break;

	case Data::SPELL_BLINK: {
		// Keep picking random spots until an unoccupied floor tile, other
		// than the player's own, turns up - the original loops
		// unconditionally until it finds one; a 500-attempt cap is added
		// here for safety, matching MapDungeon::dungeonSpawnMonster
		int newX = 0, newY = 0;
		bool valid = false;

		for (int attempt = 0; attempt < 500 && !valid; ++attempt) {
			newX = getRandomNumber(1, 9);
			newY = getRandomNumber(1, 9);
			if (newX == pos.x && newY == pos.y)
				continue;

			Data::DungeonTileId tile = dungeon._cells[newY][newX]._tileNum;
			if (tile == Data::DTILE_BEAMS || tile == Data::DTILE_WALL || tile == Data::DTILE_SECRET_DOOR)
				continue;
			if (dungeon._cells[newY][newX]._monsterId != Data::DUNGEON_NO_MONSTER)
				continue;

			valid = true;
		}

		if (valid) {
			sg._locationPosition = Common::Point(newX, newY);
			writeString("Teleported!\n");
		} else {
			failed = true;
		}
		break;
	}

	case Data::SPELL_CREATE: {
		int x = pos.x + getDirDeltaX();
		int y = pos.y + getDirDeltaY();
		Data::DungeonCell &cell = dungeon._cells[y][x];

		if (cell._tileNum == Data::DTILE_HALLWAY && cell._monsterId == Data::DUNGEON_NO_MONSTER &&
				cell._itemId == Data::DITEM_NONE) {
			cell._tileNum = Data::DTILE_BEAMS;
			writeString("Field created!\n");
		} else {
			failed = true;
		}
		break;
	}

	case Data::SPELL_DESTROY: {
		int x = pos.x + getDirDeltaX();
		int y = pos.y + getDirDeltaY();
		Data::DungeonCell &cell = dungeon._cells[y][x];

		if (cell._tileNum == Data::DTILE_BEAMS) {
			cell._tileNum = Data::DTILE_HALLWAY;
			writeString("Field destroyed!\n");
		} else {
			failed = true;
		}
		break;
	}

	case Data::SPELL_KILL: {
		int x = pos.x + getDirDeltaX();
		int y = pos.y + getDirDeltaY();

		if (dungeon._cells[y][x]._monsterId == Data::DUNGEON_NO_MONSTER)
			failed = true;
		else
			dungeonAttackAt(5, 101, 10000, x, y);
		break;
	}

	default:
		break;
	}

	if (failed) {
		playFX(6);
		writeString("Failed!\n");
	} else if (spell != Data::SPELL_KILL && spell != Data::SPELL_MAGIC_MISSILE) {
		playFX(5);
	}
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
