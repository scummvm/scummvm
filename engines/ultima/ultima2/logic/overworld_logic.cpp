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

#include "ultima/ultima2/logic/overworld_logic.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

namespace {

struct SpawnEntry {
	Data::TileId tile;
	byte hp;
	Data::TileId requiredTerrain;
};

// Indexed by the lowest set bit (0-7) of Savegame::_monsterSpawnCounter;
// an all-zero counter spawns a Balron instead (see trySpawnMonster)
const SpawnEntry SPAWN_TABLE[8] = {
	{ Data::TILE_ORC,         0x10, Data::TILE_GRASS },
	{ Data::TILE_THIEF,       0x20, Data::TILE_GRASS },
	{ Data::TILE_DAEMON,      0x40, Data::TILE_GRASS },
	{ Data::TILE_SEA_MONSTER, 0x40, Data::TILE_WATER },
	{ Data::TILE_FIGHTER,     0x80, Data::TILE_GRASS },
	{ Data::TILE_SHIP,        0xA0, Data::TILE_WATER },
	{ Data::TILE_DEVIL,       0xC0, Data::TILE_GRASS },
	{ Data::TILE_MAGE,        0xE0, Data::TILE_GRASS }
};

// The moongate's four waypoints in each era
const byte MOONGATE_WAYPOINTS[5][4][2] = {
	{ { 29, 56 }, { 31, 56 }, { 33, 56 }, { 35, 56 } },
	{ { 34, 8 }, { 47, 28 }, { 36, 56 }, { 20, 37 } },
	{ { 16, 16 }, { 48, 24 }, { 42, 24 }, { 19, 52 } },
	{ { 56, 52 }, { 28, 12 }, { 33, 23 }, { 20, 52 } },
	{ { 18, 23 }, { 50, 34 }, { 52, 24 }, { 8, 11 } }
};

} // namespace

bool OverworldLogic::isWalkable(Data::TileId tile) const {
	switch (tile) {
	case Data::TILE_WATER:
	case Data::TILE_MOUNTAIN:
		return false;
	case Data::TILE_HORSE:
	case Data::TILE_SHIP:
	case Data::TILE_AIRPLANE:
	case Data::TILE_ROCKET:
	case Data::TILE_SHIELD:
	case Data::TILE_SWORD:
	case Data::TILE_FORCEFIELD:
	case Data::TILE_ROAD:
		return true;
	default:
		return tile <= Data::TILE_SIGNPOST;
	}
}

bool OverworldLogic::mountCanEnter(Data::TileId dest) const {
	switch (_G(savegame)._mount) {
	case Data::TILE_ROCKET:
		return false;
	case Data::TILE_SHIP:
		return dest == Data::TILE_WATER;
	case Data::TILE_HORSE:
		return isWalkable(dest) && dest != Data::TILE_SWAMP;
	case Data::TILE_AIRPLANE:
		return isWalkable(dest) && dest != Data::TILE_SWAMP && dest != Data::TILE_FOREST;
	default:
		return isWalkable(dest);
	}
}

OverworldLogic::StepResult OverworldLogic::stepOnto(int x, int y) {
	Data::Savegame &sg = _G(savegame);
	Data::TileId dest = _G(map).tileAt(x, y);

	if (dest == Data::TILE_SWAMP) {
		if (!sg.deductHP(5)) {
			playerDied();
			return STEP_DIED;
		}
	} else if (dest == Data::TILE_FORCEFIELD) {
		if (sg._items[Data::ITEM_RING] != 0) {
			writeString("\nRING PROTECTS FROM FIELD!");
		} else {
			writeString("\nFIELD CAUSES 1000 DAMAGE!");
			if (!sg.deductHP(1000)) {
				playerDied();
				return STEP_DIED;
			}
		}
	}

	if (sg._legParalysisTurns > 0) {
		writeString("--PARALIZED!\n");
		return STEP_BLOCKED;
	}

	if (sg._mapType == 0 && dest == Data::TILE_MOONGATE &&
			sg._mount != Data::TILE_SHIP && sg._mount != Data::TILE_ROCKET) {
		int cost = (sg._mount == Data::TILE_HORSE || sg._mount == Data::TILE_AIRPLANE) ? 50 : 25;
		if (!sg.deductFood(cost)) {
			playerDied();
			return STEP_DIED;
		}

		return STEP_MOONGATE;
	}

	// Ships and rockets don't eat; horses and planes eat twice as much
	int foodCost = 25;
	if (sg._mount == Data::TILE_SHIP || sg._mount == Data::TILE_ROCKET)
		foodCost = 0;
	else if (sg._mount == Data::TILE_HORSE || sg._mount == Data::TILE_AIRPLANE)
		foodCost = 50;

	if (foodCost != 0 && !sg.deductFood(foodCost)) {
		playerDied();
		return STEP_DIED;
	}

	if ((!_G(intangible) && !mountCanEnter(dest)) || isOccupied(x, y))
		return STEP_BLOCKED;

	return STEP_OK;
}

bool OverworldLogic::move(Data::Direction dir) {
	Data::Savegame &sg = _G(savegame);

	int newX = sg._mapX, newY = sg._mapY;
	const char *dirName;
	switch (dir) {
	case Data::DIR_UP:
		newY = (newY + Data::MAP_HEIGHT - 1) % Data::MAP_HEIGHT;
		dirName = "NORTH";
		break;
	case Data::DIR_DOWN:
		newY = (newY + 1) % Data::MAP_HEIGHT;
		dirName = "SOUTH";
		break;
	case Data::DIR_LEFT:
		newX = (newX + Data::MAP_WIDTH - 1) % Data::MAP_WIDTH;
		dirName = "WEST";
		break;
	case Data::DIR_RIGHT:
		newX = (newX + 1) % Data::MAP_WIDTH;
		dirName = "EAST";
		break;
	default:
		return true;
	}

	playFX(Data::SFX_STEP);
	writeString("%s", dirName);

	switch (stepOnto(newX, newY)) {
	case STEP_DIED:
		return false;
	case STEP_BLOCKED:
		writeString("--INVALID MOVE!\n");
		return true;
	case STEP_MOONGATE:
		writeString("\n");
		enterMoongate(newX, newY);
		return true;
	default:
		break;
	}

	writeString("\n");
	sg._mapX = newX;
	sg._mapY = newY;
	_monstersSkipTurn = !_monstersSkipTurn;
	return true;
}

void OverworldLogic::enterMoongate(int x, int y) {
	Data::Savegame &sg = _G(savegame);
	_G(map)._tiles[y][x] = (Data::TileId)sg._patrolTerrain;

	// Each waypoint leads to a different one of the other eras, arriving
	// at that era's moongate for the same waypoint
	int waypoint = sg._patrolWaypoint >> 1;
	int era = waypoint;
	if (era >= sg._mapEra)
		++era;

	sg._mapEra = era;
	_G(map).load(era, 0);

	sg._mapX = MOONGATE_WAYPOINTS[era][waypoint][0];
	sg._mapY = MOONGATE_WAYPOINTS[era][waypoint][1];
	redrawMap();
}

void OverworldLogic::updatePatrolMarker() {
	Data::Savegame &sg = _G(savegame);
	if (sg.saveDisabled() || --sg._patrolTimer != 0)
		return;

	auto moongateAt = [&](int waypoint) -> Data::TileId & {
		const byte *pos = MOONGATE_WAYPOINTS[sg._mapEra][waypoint];
		return _G(map)._tiles[pos[1]][pos[0]];
	};

	Data::TileId &oldTile = moongateAt(sg._patrolWaypoint >> 1);
	if (oldTile == Data::TILE_MOONGATE)
		oldTile = (Data::TileId)sg._patrolTerrain;

	sg._patrolTimer = 8;
	sg._patrolWaypoint = (sg._patrolWaypoint + 2) & 7;

	// Only bare terrain (water up to mountains) can host it
	Data::TileId &newTile = moongateAt(sg._patrolWaypoint >> 1);
	if (newTile > Data::TILE_MOUNTAIN)
		return;

	sg._patrolTerrain = newTile;
	newTile = Data::TILE_MOONGATE;
}

int OverworldLogic::signByte(int v) const {
	int8 b = (int8)(byte)(v & 0xFF);
	return (b > 0) - (b < 0);
}

int OverworldLogic::findTargetMonster(int x, int y) const {
	Data::MapMonsters &monsters = _G(map)._monsters;
	for (int slot = 31; slot >= 1; --slot) {
		if (monsters.isActive(slot) && monsters._mapX[slot] == x && monsters._mapY[slot] == y)
			return slot;
	}

	return -1;
}

bool OverworldLogic::monsterCanEnter(Data::TileId monsterTile, Data::TileId destTile, int x, int y) const {
	if (monsterTile == Data::TILE_SEA_MONSTER || monsterTile == Data::TILE_SHIP)
		return destTile == Data::TILE_WATER;

	return isWalkable(destTile) && !isOccupied(x, y);
}

bool OverworldLogic::isOccupied(int x, int y) const {
	Data::MapMonsters &monsters = _G(map)._monsters;
	for (int slot = 0; slot < Data::MAP_MONSTER_COUNT; ++slot) {
		if (monsters.isActive(slot) && monsters._mapX[slot] == x && monsters._mapY[slot] == y)
			return true;
	}

	return false;
}

bool OverworldLogic::tryMoveMonster(int slot, int dx, int dy) {
	if (dx == 0 && dy == 0)
		return false;

	Data::MapMonsters &monsters = _G(map)._monsters;
	int newX = (monsters._mapX[slot] + dx + Data::MAP_WIDTH) % Data::MAP_WIDTH;
	int newY = (monsters._mapY[slot] + dy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;

	Data::TileId destTile = _G(map).tileAt(newX, newY);
	if (!monsterCanEnter(monsters.tileType(slot), destTile, newX, newY))
		return false;

	monsters._mapX[slot] = newX;
	monsters._mapY[slot] = newY;
	return true;
}

void OverworldLogic::updateCreatures() {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;
	byte damageAccumulator = 0;
	int engagedCount = 0, hitCount = 0;

	for (int slot = 31; slot >= 1; --slot) {
		if (!monsters.isActive(slot))
			continue;

		// Riding a horse, everything but sea creatures and Balrons moves every other step
		Data::TileId monsterTile = monsters.tileType(slot);
		if (sg._mount == Data::TILE_HORSE && _monstersSkipTurn && monsterTile != Data::TILE_SHIP &&
				monsterTile != Data::TILE_BALRON && monsterTile != Data::TILE_SEA_MONSTER)
			continue;

		int dx = sg._mapX - monsters._mapX[slot];
		int dy = sg._mapY - monsters._mapY[slot];
		int sdx = signByte(dx * 4);
		int sdy = signByte(dy * 4);

		// Below 15 HP, a monster flees the player instead of approaching
		if (monsters._spellHP[slot] < 15) {
			sdx = -sdx;
			sdy = -sdy;
		}

		int newX = (monsters._mapX[slot] + sdx + Data::MAP_WIDTH) % Data::MAP_WIDTH;
		int newY = (monsters._mapY[slot] + sdy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;

		if (newX == sg._mapX && newY == sg._mapY) {
			++engagedCount;

			if (monsters.tileType(slot) == Data::TILE_THIEF && randByte() < 0x40) {
				int itemIdx = randByte() & 0xF;
				if (sg._items[itemIdx] > 0) {
					--sg._items[itemIdx];
					writeString("A THIEF STOLE SOMETHING!\n");
				}
			}

			int roll = randByte();
			if (roll < 0x80 && (roll & 7) >= sg._readiedArmor) {
				++hitCount;
				int contribution = (monsters._spellHP[slot] >> 2) + 1 + damageAccumulator;
				if (_G(map).tileAt(sg._mapX, sg._mapY) == Data::TILE_TOWN)
					contribution *= 2;
				damageAccumulator = (byte)contribution;
			}
		} else {
			if (monsters._spellHP[slot] < 16)
				++monsters._spellHP[slot];

			// Try diagonal, then vertical-only, then horizontal-only
			if (!tryMoveMonster(slot, sdx, sdy) && !tryMoveMonster(slot, 0, sdy))
				tryMoveMonster(slot, sdx, 0);
		}
	}

	// A cannon-ish sound plays for every monster occupying the player's
	// tile this turn; the hit flash only for those that actually connected
	for (int i = 0; i < engagedCount; ++i) {
		playFX(Data::SFX_CANNON);
		if (i < hitCount)
			showAttackTile(sg._mapX, sg._mapY);
	}

	if (damageAccumulator != 0) {
		int dmg = (randByte() & damageAccumulator & 0x77) + 1;
		if (!sg.deductHP(dmg)) {
			playerDied();
			return;
		}
	}

	trySpawnMonster();
	updatePatrolMarker();
}

void OverworldLogic::trySpawnMonster() {
	Data::MapMonsters &monsters = _G(map)._monsters;

	int slot = -1;
	for (int i = 31; i >= 1; --i) {
		if (!monsters.isActive(i)) {
			slot = i;
			break;
		}
	}
	if (slot < 0)
		return;

	int r = randByte();
	if (r >= 0x3F)
		return;
	int x = r;

	int r2 = randByte();
	if (r2 & 0x80)
		return;
	int y = r2 & 0x3F;

	Data::Savegame &sg = _G(savegame);
	Data::TileId terrain = _G(map).tileAt(x, y);

	++sg._monsterSpawnCounter;
	int bit = -1;
	for (int i = 0; i < 8; ++i) {
		if (sg._monsterSpawnCounter & (1 << i)) {
			bit = i;
			break;
		}
	}

	Data::TileId newTile = Data::TILE_BALRON;
	byte newHp = 0xFF;
	Data::TileId requiredTerrain = Data::TILE_GRASS;
	if (bit >= 0) {
		newTile = SPAWN_TABLE[bit].tile;
		newHp = SPAWN_TABLE[bit].hp;
		requiredTerrain = SPAWN_TABLE[bit].requiredTerrain;
	}

	if (terrain != requiredTerrain) {
		// A failed attempt doesn't advance the type rotation
		--sg._monsterSpawnCounter;
		return;
	}

	monsters._mapX[slot] = x;
	monsters._mapY[slot] = y;
	monsters._spellHP[slot] = newHp;
	monsters._type[slot] = newTile * 4;
	monsters._glyphTile[slot] = 0;
	monsters._offerFlag[slot] = 0;
	monsters._tempX[slot] = 0;
	monsters._tempY[slot] = 0;
}

bool OverworldLogic::attack(Data::Direction dir) {
	Data::Savegame &sg = _G(savegame);

	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("ATTACK--");

		if (sg._armParalysisTurns > 0) {
			writeString("PARALIZED!\n");
			return true;
		}

		_directionPurpose = DirectionPurpose::ATTACK;
		g_engine->addView("Direction");
		return false;
	}

	playFX(Data::SFX_ATTACK);

	int dx = 0, dy = 0;
	switch (dir) {
	case Data::DIR_UP: dy = -1; break;
	case Data::DIR_DOWN: dy = 1; break;
	case Data::DIR_LEFT: dx = -1; break;
	case Data::DIR_RIGHT: dx = 1; break;
	default: break;
	}

	int tx = (sg._mapX + dx + Data::MAP_WIDTH) % Data::MAP_WIDTH;
	int ty = (sg._mapY + dy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;
	int slot = findTargetMonster(tx, ty);

	if (slot < 0 || (randByte() >> 1) >= sg._agility) {
		writeString("--MISS\n");
		resumeTurn();
		return false;
	}

	playFX(Data::SFX_HIT);
	writeString("--HIT!!!\n");
	alertTownGuards(slot);

	int dmg = (sg._readiedWeapon * 8 + sg._strength) >> 2;
	Data::MapMonsters &monsters = _G(map)._monsters;
	byte &hp = monsters._spellHP[slot];

	if (hp <= dmg) {
		hp = 0;
		killMonster(slot);
	} else {
		hp -= dmg + 1;
		if (monsters.tileType(slot) == Data::TILE_MINAX)
			minaxFlees(slot);
	}

	resumeTurn();
	return false;
}

bool OverworldLogic::fire(Data::Direction dir) {
	Data::Savegame &sg = _G(savegame);

	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("FIRE");

		if (sg._mount != Data::TILE_SHIP) {
			writeString(" WHAT?\n");
			return true;
		}

		writeString(" DIRECT-");
		_directionPurpose = DirectionPurpose::FIRE;
		g_engine->addView("Direction");
		return false;
	}

	int dx = 0, dy = 0;
	switch (dir) {
	case Data::DIR_UP: dy = -1; break;
	case Data::DIR_DOWN: dy = 1; break;
	case Data::DIR_LEFT: dx = -1; break;
	case Data::DIR_RIGHT: dx = 1; break;
	default: break;
	}

	int tx = (sg._mapX + dx + Data::MAP_WIDTH) % Data::MAP_WIDTH;
	int ty = (sg._mapY + dy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;
	int slot = findTargetMonster(tx, ty);

	if (slot < 0) {
		writeString("--MISS\n");
		return true;
	}

	// The cannon always hits, for 32-255 damage
	int dmg = randByte() | 0x20;
	byte &hp = _G(map)._monsters._spellHP[slot];

	writeString("\n");
	if (hp <= dmg) {
		hp = 0;
		killMonster(slot);
	} else {
		hp -= dmg + 1;
		if (_G(map)._monsters.tileType(slot) == Data::TILE_MINAX)
			minaxFlees(slot);
	}

	return true;
}

void OverworldLogic::minaxFlees(int slot) {
	Data::MapMonsters &monsters = _G(map)._monsters;

	// She reappears at the mirror image of her current position, and never
	// budges again short of being killed
	byte oldX = monsters._mapX[slot], oldY = monsters._mapY[slot];
	monsters._mapX[slot] = oldY;
	monsters._mapY[slot] = oldX;
	monsters._offerFlag[slot] = 0x84;

	writeString("\nSHE'S GONE!!!\n");
}

void OverworldLogic::minaxDeathSequence() {
	writeString("\n\n     MINAX IS DEAD!!\nALL HER WORKS SHALL DIE!\n");

	// The castle crumbles into rubble before the screen hands off to the
	// endless victory pan
	Shared::UIElement *curView = g_engine->focusedView();
	for (int step = 0; step < 64; ++step) {
		for (int i = 0; i < 64; ++i) {
			int x = randByte() & 0x3F, y = randByte() & 0x3F;
			_G(map)._tiles[y][x] = Data::TILE_EMPTY;
		}

		curView->draw();
		g_engine->updateScreen();
		g_engine->pauseMillis(30);
	}

	writeString("\nYOU FEEL A STRANGE FORCE!\n");
	g_engine->addView("Ending");
}

void OverworldLogic::killMonster(int slot) {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;
	Data::TileId monsterTile = monsters.tileType(slot);
	monsters._type[slot] = 0;

	if (monsterTile == Data::TILE_MINAX) {
		minaxDeathSequence();
		return;
	}

	switch (monsterTile) {
	case Data::TILE_GUARD:
		sg._keys += 2;
		break;

	case Data::TILE_THIEF: {
		if (randByte() < 0x40)
			++sg._thievesTools;
		int itemIdx = randByte() & 0xF;
		if (itemIdx != 0)
			++sg._items[itemIdx];
		break;
	}

	case Data::TILE_FIGHTER:
		if (randByte() < 0x40)
			++sg._items[Data::ITEM_HELM];
		sg._torches += (randByte() & 3) + 1;
		break;

	case Data::TILE_MAGE:
		if ((randByte() & 1) == 0)
			++sg._items[Data::ITEM_WAND];
		else
			++sg._items[Data::ITEM_STAFF];
		break;

	default:
		break;
	}

	// The original keeps and prints these as two BCD digits
	int goldAmt = Data::bcdValue((randByte() & 0x17) | 1);
	int expAmt = (randByte() & 3) + 1;
	sg._gold += goldAmt;
	sg._experience += expAmt;
	writeString("KILLED--GOLD+%.2d--EXP.+%.2d\n", goldAmt, expAmt);
}

void OverworldLogic::enterLocalMap(int mapType) {
	Data::Savegame &sg = _G(savegame);
	sg._overworldReturnX = sg._mapX;
	sg._overworldReturnY = sg._mapY;
	sg._mapX = 31;
	sg._mapY = 62;
	sg._mapType = mapType;
	_G(map).load(sg._mapEra, sg._mapType);
}

void OverworldLogic::enterDungeon(int mapType) {
	Data::Savegame &sg = _G(savegame);
	sg._overworldReturnX = sg._mapX;
	sg._overworldReturnY = sg._mapY;
	sg._mapX = 5;
	sg._mapY = 5;
	sg._dungeonLevel = 0;
	sg._facingX = 1;
	sg._facingY = 0;
	sg._mapType = mapType;
	_G(map).load(sg._mapEra, sg._mapType);
	_G(logic)->entering();
}

bool OverworldLogic::enter() {
	Data::Savegame &sg = _G(savegame);
	writeString("ENTER");

	if (sg._mapType != 0) {
		writeString(" WHAT?\n");
		return true;
	}

	if (sg._mount != 0) {
		writeString("-ONLY ON FOOT!\n");
		return true;
	}

	Data::TileId tile = _G(map).tileAt(sg._mapX, sg._mapY);
	switch (tile) {
	case Data::TILE_VILLAGE:
		writeString("-VILLAGE\n");
		enterLocalMap(1);
		return true;
	case Data::TILE_TOWN:
		writeString("-TOWN\n");
		enterLocalMap(2);
		return true;
	case Data::TILE_CASTLE:
		writeString("-CASTLE\n");
		enterLocalMap(3);
		return true;
	case Data::TILE_TOWER:
		writeString("-TOWER\n");
		enterDungeon(4);
		return true;
	case Data::TILE_DUNGEON_ENTRANCE:
		writeString("-DUNGEON\n");
		enterDungeon(5);
		return true;
	case Data::TILE_SIGNPOST: {
		static const char *const ERA_TEXT[5] = {
			"ANOS: LEGENDS!", "ANOS: 9,000,000 B.C.", "ANOS: 1423 B.C.",
			"ANOS: 1990 A.D.", "ANOS: 2112 A.D."
		};
		int era = (sg._mapEra < 5) ? sg._mapEra : 4;
		writeString("-THE SIGN READS:\n%s\n", ERA_TEXT[era]);
		return true;
	}
	default:
		writeString(" WHAT?\n");
		return true;
	}
}

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima
