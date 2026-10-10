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

#include "ultima/ultima3/logic/combat_logic.h"
#include "ultima/ultima3/logic/location_logic.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {

using Data::ARENA_SIZE;
using Data::ARENA_MONSTERS;
using Data::ARENA_ABSENT;

constexpr byte TILE_GRASS = 1;
constexpr byte TILE_BRUSH = 2;
constexpr byte TILE_FOREST = 3;
constexpr byte TILE_FLOOR = 8;
constexpr byte TILE_CHEST = 9;
constexpr byte TILE_FORCE_FIELD = 0x20;
constexpr byte TILE_LAVA = 0x21;
constexpr byte TILE_SHOT = 0x3D;
constexpr byte TILE_SPELL_SHOT = 0x3C;

constexpr byte CELL_CHEST = 0x24;
constexpr byte CELL_SHIP = 0x2C;
constexpr byte CELL_GUARD = 0x48;
constexpr byte CELL_ROYAL_GUARD = 0x4C;
constexpr byte FLAGS_HOSTILE = 0xC0;

// The monsters that matter for how they behave
constexpr byte MONSTER_PIRATE = 0x0F;
constexpr byte MONSTER_THIEF = 0x17;
constexpr byte MONSTER_LORD_BRITISH = 0x13;
constexpr byte MONSTER_DRAGON = 0x1D;

constexpr int ALTERNATE_NAMES = 0x79;
constexpr byte EXODUS_ENTRANCE_X = 10;
constexpr byte EXOTIC_WEAPON = 0x0F;
constexpr byte EXOTIC_ARMOUR = 7;

int CombatLogic::rollBelow(int limit) {
	return Graphics::Views::g_events->getRandomNumber(limit - 1);
}

void CombatLogic::print(const Common::String &text) {
	_events.push_back(CombatEvent(text));
}

void CombatLogic::sound(int effect) {
	_events.push_back(CombatEvent(CombatEvent::SOUND, effect));
}

void CombatLogic::overlay(int x, int y, int tile) {
	_events.push_back(CombatEvent(CombatEvent::OVERLAY, x, y, tile));
}

void CombatLogic::flashSlot(int slot) {
	_events.push_back(CombatEvent(CombatEvent::FLASH_SLOT, slot));
}

void CombatLogic::show() {
	_events.push_back(CombatEvent(CombatEvent::SHOW));
}

void CombatLogic::syncShown() {
	memcpy(_shown, _arena._tiles, sizeof(_shown));
}

bool CombatLogic::isExodusFight() const {
	return _savedLocation == Data::LOCATION_CASTLE && _G(savegame)._worldX == EXODUS_ENTRANCE_X;
}

Common::String CombatLogic::monsterName(bool heading) const {
	Data::Savegame &save = _G(savegame);
	int name;

	// People and things of the kinds higher up the list go by two names
	if ((save._posY & 1) && _monsterClass >= MONSTER_THIEF) {
		name = ALTERNATE_NAMES + (((_monsterClass - MONSTER_THIEF) << 1) | (save._posX & 1));
	} else if (_monsterClass == MONSTER_THIEF && heading) {
		return "Thieves\n\n";
	} else {
		name = _monsterClass + 1;
	}

	Common::String result = Data::NAMES[name - 1];

	if (heading) {
		switch (name) {
		case 2:
		case 9:
		case 0x14:
		case 0x87:
			result += "\n\n";
			break;
		case 0x83:
			result += "es\n\n";
			break;
		default:
			result += "s\n\n";
			break;
		}
	}

	return result;
}

const char *CombatLogic::chooseArena() {
	Data::Savegame &save = _G(savegame);
	const bool onShip = save._transport == TRANSPORT_SHIP;

	if (_savedLocation == Data::LOCATION_DUNGEON)
		return "CNFLCT_C.ULT";

	if (_monsterClass == MONSTER_PIRATE) {
		_monsterClass = MONSTER_THIEF;
		return onShip ? "CNFLCT_S.ULT" : "CNFLCT_A.ULT";
	}

	if (onShip)
		return _monsterClass < 0x10 ? "CNFLCT_Q.ULT" : "CNFLCT_R.ULT";

	if (_monsterClass >= 0x0B && _monsterClass < 0x10)
		return "CNFLCT_M.ULT";

	// Otherwise the ground the party is on decides
	int terrain = _G(map).tile(save._posX, save._posY);
	if (terrain == TILE_CHEST) {
		terrain = _G(map).cell(save._posX, save._posY) & 3;
		if (terrain == 0)
			terrain = TILE_FLOOR;
	}

	switch (terrain) {
	case TILE_GRASS:
		return "CNFLCT_G.ULT";
	case TILE_BRUSH:
		return "CNFLCT_B.ULT";
	case TILE_FOREST:
		return "CNFLCT_F.ULT";
	case TILE_FLOOR:
	case TILE_FORCE_FIELD:
	case TILE_LAVA:
		return "CNFLCT_C.ULT";
	default:
		return _monsterClass == TILE_FLOOR ? "CNFLCT_C.ULT" : "CNFLCT_G.ULT";
	}
}

void CombatLogic::begin(int creature) {
	Data::Savegame &save = _G(savegame);
	Data::Map &map = _G(map);
	Data::Creatures &c = map._creatures;

	// Whatever the creature was standing on is left behind as a chest
	int x = c._x[creature], y = c._y[creature];
	byte floor = c._floor[creature];
	map.setCell(x, y, floor ? (((floor >> 2) & 3) | CELL_CHEST) : 0);

	_monsterClass = c._tile[creature] >> 2;
	c._tile[creature] = 0;
	if (_monsterClass == MONSTER_PIRATE && save._transport != TRANSPORT_SHIP)
		map.setCell(x, y, CELL_SHIP);

	startFight();
}

void CombatLogic::beginDungeon(byte monsterClass) {
	_monsterClass = monsterClass;
	startFight();
}

void CombatLogic::startFight() {
	Data::Savegame &save = _G(savegame);
	Data::Creatures &c = _G(map)._creatures;

	// Guards turn on the party
	for (int i = 0; i < Data::CREATURE_COUNT; ++i) {
		if (c._tile[i] == CELL_GUARD || c._tile[i] == CELL_ROYAL_GUARD)
			c._flags[i] = FLAGS_HOSTILE;
	}

	_savedLocation = save._location;
	_events.clear();
	_G(holdTime) = 0;
	_slayUsed = false;
	_combatant = 0;

	_G(messages).print("\n---Conflict!!---\n->");
	_G(messages).print(monsterName(true).c_str());
	_G(messages).print("\n\n");

	_arena.load(chooseArena());

	// Everyone able to fight takes their place
	for (int slot = 0; slot < Data::PARTY_MAX; ++slot) {
		if (slot < save._partySize && save.partyMember(slot).isAlive()) {
			const Data::RosterEntry &e = save.partyMember(slot);

			_arena._playerTile[slot] = Data::fightingTile(e._class);
			_arena._playerUnder[slot] = _arena.tile(_arena._playerX[slot], _arena._playerY[slot]);
			_arena.setTile(_arena._playerX[slot], _arena._playerY[slot], _arena._playerTile[slot]);
		} else {
			_arena._playerX[slot] = _arena._playerY[slot] = ARENA_ABSENT;
		}
	}

	int count;
	if (isExodusFight())
		count = ARENA_MONSTERS;
	else if (_savedLocation >= Data::LOCATION_TOWN && _savedLocation != 0xFF)
		count = (_monsterClass == 0x12) ? ARENA_MONSTERS : 1;
	else
		count = rollBelow(ARENA_MONSTERS) + 1;

	while (count-- > 0) {
		int slot;
		do {
			slot = rollBelow(ARENA_MONSTERS);
		} while (_arena._monsterHp[slot] != 0);

		int maxHp = Data::MONSTER_HIT_POINTS[_monsterClass & 0xF];
		_arena._monsterHp[slot] = rollBelow(maxHp) | 0x0F;

		int mx = _arena._monsterX[slot], my = _arena._monsterY[slot];
		_arena._monsterUnder[slot] = _arena.tile(mx, my);
		_arena.setTile(mx, my, _monsterClass);
	}

	syncShown();
}

int CombatLogic::negateTurns() const {
	return _G(holdTime);
}

void CombatLogic::decrementHold() {
	--_G(holdTime);
}

void CombatLogic::endHold() {
	_G(holdTime) = 0;
}

bool CombatLogic::allMonstersDead() const {
	for (int i = 0; i < ARENA_MONSTERS; ++i) {
		if (_arena._monsterHp[i])
			return false;
	}

	return true;
}

int CombatLogic::monsterAt(int x, int y) const {
	for (int i = ARENA_MONSTERS - 1; i >= 0; --i) {
		if (_arena._monsterHp[i] && _arena._monsterX[i] == x && _arena._monsterY[i] == y)
			return i;
	}

	return -1;
}

bool CombatLogic::movePlayer(int slot, int dx, int dy) {
	int x = (byte)(_arena._playerX[slot] + dx);
	int y = (byte)(_arena._playerY[slot] + dy);
	if (x >= ARENA_SIZE || y >= ARENA_SIZE)
		return false;

	byte tile = _arena.tile(x, y);
	if (tile != TILE_GRASS && tile != TILE_BRUSH && tile != TILE_FOREST && tile != TILE_FLOOR)
		return false;

	sound(0xF6);
	_arena.setTile(_arena._playerX[slot], _arena._playerY[slot], _arena._playerUnder[slot]);
	_arena._playerUnder[slot] = tile;
	_arena.setTile(x, y, _arena._playerTile[slot]);
	_arena._playerX[slot] = x;
	_arena._playerY[slot] = y;
	show();
	return true;
}

int CombatLogic::shoot(int x, int y, int dx, int dy, int tile) {
	for (;;) {
		x += dx;
		y += dy;
		if ((byte)y >= ARENA_SIZE || (byte)x >= ARENA_SIZE)
			return -1;

		overlay(x, y, tile);

		int monster = monsterAt(x, y);
		if (monster >= 0)
			return monster;
	}
}

void CombatLogic::playerAttack(int slot, Direction dir) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);
	const int dx = (dir == DIR_EAST) ? 1 : (dir == DIR_WEST) ? -1 : 0;
	const int dy = (dir == DIR_SOUTH) ? 1 : (dir == DIR_NORTH) ? -1 : 0;
	const int x = _arena._playerX[slot], y = _arena._playerY[slot];

	const byte weapon = e._weaponIndex;
	bool ranged = weapon == 3 || weapon == 5 || weapon == 9 || weapon == 0x0D;
	int monster = -1;

	sound(0xFD);

	// Close in, a monster has to be right beside the attacker
	if (!ranged) {
		int tx = x + dx, ty = y + dy;
		if (tx >= 0 && ty >= 0 && tx < ARENA_SIZE && ty < ARENA_SIZE)
			monster = monsterAt(tx, ty);

		// Otherwise daggers can be thrown
		if (monster < 0 && weapon == 1) {
			int left = Data::fromBcd(e._weaponOwned[0]) - 1;
			e._weaponOwned[0] = Data::toBcd(left);
			if (left == 0)
				e._weaponIndex = 0;

			ranged = true;
		}
	}

	if (ranged)
		monster = shoot(x, y, dx, dy, TILE_SHOT);

	if (monster < 0) {
		print("Missed!\n");
		return;
	}

	playerHit(slot, monster);
}

void CombatLogic::playerHit(int slot, int monster) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);

	// Only the exotic weapon harms what's found in Exodus's castle
	if (isExodusFight() && e._weaponIndex != EXOTIC_WEAPON) {
		print("Missed!\n");
		return;
	}

	bool hit = rollBelow(255) >= 128;
	if (!hit)
		hit = Data::toBcd(rollBelow(99)) < e._dexterity;

	if (!hit) {
		print("Missed!\n");
		return;
	}

	print(monsterName(false));
	print("\nHit!\n");

	overlay(_arena._monsterX[monster], _arena._monsterY[monster], TILE_SHOT);
	sound(0xF7);

	int strength = Data::fromBcd(e._strength);
	int bonus = rollBelow(strength | 1) + (e._strength >> 1) + (e._strength & 1);
	byte damage = e._weaponIndex * 3 + bonus + 4;

	damageMonster(slot, monster, damage);
}

void CombatLogic::damageMonster(int slot, int monster, int damage) {
	if (_monsterClass == MONSTER_LORD_BRITISH)
		return;

	if (damage < _arena._monsterHp[monster]) {
		_arena._monsterHp[monster] -= damage;
		return;
	}

	Data::RosterEntry &e = _G(savegame).partyMember(slot);
	byte gain = Data::MONSTER_EXPERIENCE[_monsterClass & 0xF];

	_arena._monsterHp[monster] = 0;
	_arena.setTile(_arena._monsterX[monster], _arena._monsterY[monster], _arena._monsterUnder[monster]);
	print(Common::String::format("Killed! Exp.+%02X\n", gain));

	int experience = Data::fromBcdWord(e._experience) + Data::fromBcd(gain);
	e._experience = Data::toBcdWord(MIN(experience, 9999));
	show();
}

bool CombatLogic::canMonsterMoveTo(int x, int y) const {
	if (x < 0 || y < 0 || x >= ARENA_SIZE || y >= ARENA_SIZE)
		return false;

	byte tile = _arena.tile(x, y);
	bool terrain;

	// The things of the sea stay in the water
	if (_monsterClass >= 0x0B && _monsterClass < 0x10)
		terrain = tile == 0;
	else
		terrain = tile == TILE_GRASS || tile == TILE_BRUSH || tile == TILE_FOREST || tile == TILE_FLOOR;

	return terrain && monsterAt(x, y) < 0;
}

static int sign(int value) {
	return (value > 0) - (value < 0);
}

int CombatLogic::findTarget(int monster, int &target, int &stepX, int &stepY, int &toX, int &toY) const {
	Data::Savegame &save = _G(savegame);
	const int mx = _arena._monsterX[monster], my = _arena._monsterY[monster];
	int best = 0xFF;
	target = -1;

	for (int slot = 0; slot < save._partySize; ++slot) {
		if (!save.partyMember(slot).isAlive())
			continue;

		int dx = _arena._playerX[slot] - mx;
		int dy = _arena._playerY[slot] - my;

		// Right beside someone
		if (ABS(dx) <= 1 && ABS(dy) <= 1) {
			target = slot;
			return 0;
		}

		int distance = ABS(dx) + ABS(dy);
		if (distance > best)
			continue;

		// Try heading straight for them, then along each axis in turn
		int sx = sign(dx), sy = sign(dy);
		int tx, ty;
		if (canMonsterMoveTo(mx + sx, my + sy)) {
			tx = mx + sx;
			ty = my + sy;
		} else if (canMonsterMoveTo(mx, my + sy)) {
			tx = mx;
			ty = my + sy;
		} else if (canMonsterMoveTo(mx + sx, my)) {
			tx = mx + sx;
			ty = my;
		} else {
			continue;
		}

		best = distance;
		target = slot;
		stepX = sx;
		stepY = sy;
		toX = tx;
		toY = ty;
	}

	return best;
}

void CombatLogic::killPlayer(int slot) {
	_arena.setTile(_arena._playerX[slot], _arena._playerY[slot], _arena._playerUnder[slot]);
	_arena._playerX[slot] = _arena._playerY[slot] = ARENA_ABSENT;
	print("Killed!!!\n");
	show();
}

void CombatLogic::damagePlayer(int slot, int tile) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);

	int limit = ((Data::MONSTER_HIT_POINTS[_monsterClass & 0xF] >> 3) + (e._maxHitPoints >> 8)) & 0xFF;
	int roll = rollBelow(limit | 1) + 1;

	// The amount is worked out in tens and units, which only matters for big rolls
	int amount = Data::fromBcd((byte)(((roll / 10) << 4) | (roll % 10)));
	int extra = (_savedLocation & 3) * 10;

	damageCharacter(slot, amount);
	damageCharacter(slot, extra);

	flashSlot(slot);
	overlay(_arena._playerX[slot], _arena._playerY[slot], tile);
	sound(0xF7);

	if (e._status == Data::STATUS_DEAD)
		killPlayer(slot);
}

void CombatLogic::monsterPoison(int slot) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);

	if (rollBelow(255) & 3 || e._status != Data::STATUS_GOOD)
		return;

	e._status = Data::STATUS_POISONED;
	print(Common::String::format("Plr %d Poisoned!\n", slot + 1));
	sound(0xFA);
}

void CombatLogic::monsterSteal(int slot) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);
	byte *owned;
	int pick;

	// Either a weapon or a set of armour is taken
	if (rollBelow(255) >= 128) {
		pick = rollBelow(8);
		if (pick == 0 || pick == e._armourIndex)
			return;
		owned = &e._armourOwned[pick - 1];
	} else {
		pick = rollBelow(16);
		if (pick == 0 || pick == e._weaponIndex)
			return;
		owned = &e._weaponOwned[pick - 1];
	}

	if (*owned == 0)
		return;

	*owned = Data::toBcd(Data::fromBcd(*owned) - 1);
	print(Common::String::format("Plr %d Pilfered!\n", slot + 1));
	sound(0xFA);
}

void CombatLogic::monsterAttack(int slot, int tile) {
	const Data::RosterEntry &e = _G(savegame).partyMember(slot);

	if (_monsterClass == 0x0E || _monsterClass == 0x1E || _monsterClass == 0x1C)
		monsterPoison(slot);
	else if (_monsterClass == MONSTER_THIEF)
		monsterSteal(slot);

	print(Common::String::format("Plr %d", slot + 1));
	sound(0xF8);

	bool hit = true;
	if (!isExodusFight() || e._armourIndex == EXOTIC_ARMOUR)
		hit = rollBelow(0x10 + e._armourIndex) < 8;

	if (hit) {
		print(" Hit!\n");
		damagePlayer(slot, tile);
	} else {
		print(" Missed!\n");
	}
}

void CombatLogic::monsterBreath(int monster, int stepX, int stepY) {
	Data::Savegame &save = _G(savegame);
	int x = _arena._monsterX[monster], y = _arena._monsterY[monster];

	sound(0xFB);

	for (;;) {
		x += stepX;
		y += stepY;
		if ((byte)x >= ARENA_SIZE || (byte)y >= ARENA_SIZE)
			return;

		for (int slot = save._partySize - 1; slot >= 0; --slot) {
			if (_arena._playerX[slot] == x && _arena._playerY[slot] == y) {
				damagePlayer(slot, TILE_SHOT);
				return;
			}
		}

		overlay(x, y, TILE_SHOT);
	}
}

void CombatLogic::monsterTurn(int monster) {
	Data::Savegame &save = _G(savegame);
	int target, stepX = 0, stepY = 0, toX = 0, toY = 0;
	int distance = findTarget(monster, target, stepX, stepY, toX, toY);

	// Beside one of the party
	if (distance == 0) {
		monsterAttack(target, TILE_SHOT);
		return;
	}

	// Dragons breathe fire along the line they're heading
	if (target >= 0 && rollBelow(255) < 128 && _monsterClass == MONSTER_DRAGON) {
		monsterBreath(monster, stepX, stepY);
		return;
	}

	bool move = distance != 0xFF;

	if (rollBelow(0xC0) >= 0x80) {
		switch (_monsterClass) {
		case 0x0D:
		case 0x0E:
		case 0x16:
		case 0x1B:
		case 0x1D:
		case 0x1E: {
			// Some attack from a distance, at anyone in good health
			int slot = rollBelow(255) & 3;
			if (slot < save._partySize && save.partyMember(slot)._status == Data::STATUS_GOOD) {
				_events.push_back(CombatEvent(CombatEvent::FLASH_VIEW));
				monsterAttack(slot, TILE_SPELL_SHOT);
				return;
			}
			break;
		}

		default:
			break;
		}
	}

	if (!move)
		return;

	_arena.setTile(_arena._monsterX[monster], _arena._monsterY[monster], _arena._monsterUnder[monster]);
	_arena._monsterX[monster] = toX;
	_arena._monsterY[monster] = toY;
	_arena._monsterUnder[monster] = _arena.tile(toX, toY);
	_arena.setTile(toX, toY, _monsterClass);
	show();
}

void CombatLogic::monstersTurn() {
	for (int monster = 0; monster < ARENA_MONSTERS; ++monster) {
		if (!_arena._monsterHp[monster])
			continue;
		if (!_G(savegame).hasLivingPartyMember())
			break;

		monsterTurn(monster);
	}
}

void CombatLogic::castProjectile(int slot, int damage, Direction dir) {
	const int dx = (dir == DIR_EAST) ? 1 : (dir == DIR_WEST) ? -1 : 0;
	const int dy = (dir == DIR_SOUTH) ? 1 : (dir == DIR_NORTH) ? -1 : 0;

	int monster = shoot(_arena._playerX[slot], _arena._playerY[slot], dx, dy, TILE_SPELL_SHOT);
	if (monster < 0)
		return;

	sound(0xF7);
	damageMonster(slot, monster, (byte)damage);
}

void CombatLogic::castGroupDamage(int slot, int damage) {
	for (int monster = 0; monster < ARENA_MONSTERS; ++monster) {
		if ((rollBelow(255) & 3) == 0 || !_arena._monsterHp[monster])
			continue;

		overlay(_arena._monsterX[monster], _arena._monsterY[monster], TILE_SPELL_SHOT);
		sound(0xF7);
		damageMonster(slot, monster, (byte)damage);
	}
}

void CombatLogic::castDrain() {
	for (int monster = 0; monster < ARENA_MONSTERS; ++monster) {
		if (!_arena._monsterHp[monster])
			continue;

		_arena._monsterHp[monster] = 5;
		overlay(_arena._monsterX[monster], _arena._monsterY[monster], TILE_SPELL_SHOT);
		sound(0xF7);
	}
}

bool CombatLogic::castSlay(int slot, byte monsterClass) {
	if (_monsterClass != monsterClass || _slayUsed)
		return false;

	_slayUsed = true;
	if (rollBelow(255) >= 128)
		return false;

	castGroupDamage(slot, 0xFF);
	return true;
}

} // namespace Ultima3
} // namespace Ultima
