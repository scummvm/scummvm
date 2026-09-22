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

#include "ultima/ultima2/logic/dungeon_logic.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

namespace {

constexpr int MAX_LEVEL = 15;
constexpr int TORCH_TURNS = 150;

// Cell contents: the high nibble is the terrain, the low bits flag a monster
constexpr byte CELL_WALL = 0x80;
constexpr byte CELL_CHEST = 0x40;
constexpr byte CELL_LADDER_UP = 0x10;
constexpr byte CELL_LADDER_DOWN = 0x20;

int signOf(int v) {
	return (v > 0) - (v < 0);
}

} // namespace

byte &DungeonLogic::cellAt(int x, int y) const {
	return _G(dungeon).cell(_G(savegame)._dungeonLevel, x, y);
}

int DungeonLogic::findMonsterAhead() const {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;
	byte x = sg._mapX + sg._facingX, y = sg._mapY + sg._facingY;

	for (int slot = 31; slot >= 1; --slot) {
		if (monsters.isActive(slot) && monsters._glyphTile[slot] == sg._dungeonLevel &&
				monsters._mapX[slot] == x && monsters._mapY[slot] == y)
			return slot;
	}

	return -1;
}

bool DungeonLogic::move(Data::Direction dir) {
	switch (dir) {
	case Data::DIR_UP:
		return advance();
	case Data::DIR_DOWN:
		return retreat();
	case Data::DIR_LEFT:
		return turn(true);
	case Data::DIR_RIGHT:
		return turn(false);
	default:
		return true;
	}
}

bool DungeonLogic::advance() {
	Data::Savegame &sg = _G(savegame);
	writeString("ADVANCE");

	byte front = cellAt((byte)(sg._mapX + sg._facingX), (byte)(sg._mapY + sg._facingY));
	if ((front & 0x0F) != 0 || front == CELL_WALL) {
		writeString("-BLOCKED!\n");
		return true;
	}

	sg._mapX = (sg._mapX + sg._facingX) & 0x3F;
	sg._mapY = (sg._mapY + sg._facingY) & 0x3F;

	// Deeper levels have more traps
	if (randByte() >= sg._dungeonLevel) {
		writeString("\n");
		return true;
	}

	sg._lightTurns = 0;
	playFX(Data::SFX_TRAP);
	writeString("\nARGH! A TRAP!\n");

	if (sg._thievesTools == 0) {
		playerDied();
		return false;
	}

	writeString("ESCAPED! BY USE OF TOOLS!\n");
	--sg._thievesTools;
	return true;
}

bool DungeonLogic::retreat() {
	Data::Savegame &sg = _G(savegame);
	writeString("RETREAT");

	byte back = cellAt((byte)(sg._mapX - sg._facingX), (byte)(sg._mapY - sg._facingY));
	if ((back & 0x8F) != 0) {
		writeString("-BLOCKED!\n");
		return true;
	}

	sg._mapX = (sg._mapX - sg._facingX) & 0x3F;
	sg._mapY = (sg._mapY - sg._facingY) & 0x3F;
	writeString("\n");
	return true;
}

bool DungeonLogic::turn(bool left) {
	Data::Savegame &sg = _G(savegame);
	writeString(left ? "TURN LEFT\n" : "TURN RIGHT\n");

	int8 fx = sg._facingX, fy = sg._facingY;
	if (left) {
		if (fy != 0) {
			sg._facingX = fy;
			sg._facingY = 0;
		} else {
			sg._facingY = -fx;
			sg._facingX = 0;
		}
	} else {
		if (fx != 0) {
			sg._facingY = fx;
			sg._facingX = 0;
		} else {
			sg._facingX = -fy;
			sg._facingY = 0;
		}
	}

	return true;
}

void DungeonLogic::printLevel() {
	writeString("\nTO LEVEL %02d\n", _G(savegame)._dungeonLevel);
}

void DungeonLogic::surface() {
	Data::Savegame &sg = _G(savegame);
	sg._mapX = sg._overworldReturnX;
	sg._mapY = sg._overworldReturnY;
	sg._mapType = 0;
	writeString("\n");
	_G(map).load(sg._mapEra, sg._mapType);
}

void DungeonLogic::changeLevel(int delta) {
	Data::Savegame &sg = _G(savegame);
	int level = sg._dungeonLevel + delta;

	if (level < 0) {
		surface();
	} else {
		sg._dungeonLevel = level;
		printLevel();
	}
}

bool DungeonLogic::descend() {
	Data::Savegame &sg = _G(savegame);
	writeString("DESCEND");

	if ((cellAt(sg._mapX, sg._mapY) & CELL_LADDER_DOWN) == 0) {
		writeString("-WHAT?\n");
		return true;
	}

	// Towers are numbered from the bottom up, dungeons from the top down
	changeLevel(sg._mapType == 4 ? -1 : 1);
	return true;
}

bool DungeonLogic::klimb() {
	Data::Savegame &sg = _G(savegame);
	writeString("KLIMB");

	if ((cellAt(sg._mapX, sg._mapY) & CELL_LADDER_UP) == 0) {
		writeString("-WHAT?\n");
		return true;
	}

	changeLevel(sg._mapType == 4 ? 1 : -1);
	return true;
}

bool DungeonLogic::get() {
	Data::Savegame &sg = _G(savegame);
	writeString("GET");

	byte &cell = cellAt(sg._mapX, sg._mapY);
	if (cell != CELL_CHEST) {
		writeString(" WHAT?\n");
		return true;
	}

	writeString(" CHEST!\nIT CONTAINS ");
	cell = 0;

	int level = sg._dungeonLevel;
	if (level == MAX_LEVEL) {
		writeString("TRI-LITHIUM!\n");
		++sg._items[Data::ITEM_TRI_LITHIUM];
		return true;
	}

	if (randByte() >= 0x40) {
		writeString("GOLD!\n");
		int amount = ((randByte() & 0x1F) + level * 2) & 0x77;
		sg._gold = (sg._gold + Data::bcdValue(amount)) % 10000;
		return true;
	}

	// Even levels hold weapons, odd ones armour
	if ((level & 1) == 0) {
		writeString("A  WEAPON");
		int r = randByte() & 7;
		if (r != 0) {
			++sg._weaponOwned[r];
			writeString("\n");
			return true;
		}
	} else {
		writeString(" ARMOUR");
		int r = randByte() & 3;
		if (r != 0) {
			++sg._armorOwned[r];
			writeString("\n");
			return true;
		}
	}

	writeString(" EMPTY!\n");
	return true;
}

void DungeonLogic::killMonster(int slot) {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;

	cellAt(monsters._mapX[slot], monsters._mapY[slot]) &= 0xF0;
	monsters._spellHP[slot] = 0;
	monsters._type[slot] = 0;
	monsters._glyphTile[slot] = 0;

	int gold = Data::bcdValue((randByte() & 0x17) | 1);
	writeString("\nKILLED--GOLD+%.2d", gold);
	sg._gold = (sg._gold + gold) % 10000;

	int exp = randByte() & 7;
	writeString("--EXP.+%.2d\n", exp);
	sg._experience = (sg._experience + exp) % 10000;
}

bool DungeonLogic::attack(Data::Direction dir) {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;
	playFX(Data::SFX_ATTACK);
	writeString("ATTACK--");

	byte x = sg._mapX + sg._facingX, y = sg._mapY + sg._facingY;
	if ((cellAt(x, y) & 7) == 0) {
		writeString("MISS\n");
		return true;
	}

	int slot = -1;
	for (int i = 31; i >= 0; --i) {
		if (monsters.isActive(i) && monsters._mapX[i] == x && monsters._mapY[i] == y &&
				monsters._glyphTile[i] == sg._dungeonLevel) {
			slot = i;
			break;
		}
	}

	if (slot < 0 || (randByte() >> 1) >= sg._agility) {
		writeString("MISS\n");
		return true;
	}

	playFX(Data::SFX_HIT);
	writeString("HIT!");
	int dmg = (sg._readiedWeapon * 8 + sg._strength) >> 2;
	byte &hp = monsters._spellHP[slot];

	if (hp < dmg) {
		killMonster(slot);
	} else {
		hp -= dmg;
		writeString("\n");
	}

	return true;
}

bool DungeonLogic::castSpell(Data::SpellType spell) {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;
	int level = sg._dungeonLevel;

	switch (spell) {
	case Data::SPELL_LIGHT:
		sg._lightTurns = TORCH_TURNS;
		writeString("\n");
		return true;

	case Data::SPELL_DOWN_LADDER:
	case Data::SPELL_UP_LADDER: {
		// Down Ladder goes deeper in a dungeon but shallower in a tower, and
		// Up Ladder the reverse
		bool deeper = (spell == Data::SPELL_DOWN_LADDER) == (sg._mapType == 5);
		if (!deeper && level == 0) {
			surface();
			return true;
		}

		int dest = deeper ? level + 1 : level - 1;
		if (dest > MAX_LEVEL || _G(dungeon).cell(dest, sg._mapX, sg._mapY) != 0)
			break;

		sg._dungeonLevel = dest;
		printLevel();
		return true;
	}

	case Data::SPELL_PASSWALL: {
		int x = (sg._mapX + sg._facingX) & 0xF, y = (sg._mapY + sg._facingY) & 0xF;
		if (x == 0 || y == 0 || (cellAt(x, y) & CELL_WALL) == 0)
			break;

		cellAt(x, y) = 0;
		writeString("\n");
		return true;
	}

	case Data::SPELL_SURFACE:
		surface();
		return true;

	case Data::SPELL_PRAYER:
	case Data::SPELL_KILL:
	case Data::SPELL_MAGIC_MISSILE: {
		if (spell == Data::SPELL_PRAYER && (randByte() & 0x80) != 0)
			break;

		int slot = findMonsterAhead();
		if (slot < 0)
			break;

		if (spell == Data::SPELL_MAGIC_MISSILE) {
			// Damage grows with the hundreds of experience
			int exp = Data::toBcd((sg._experience / 100) % 100);
			int sum = exp + exp;
			int carry = sum >> 8;
			sum = (sum & 0xFF) + 0x1E + carry;
			int dmg = sum & 0xFF;
			int borrow = (sum >> 8) ? 0 : 1;

			byte &hp = monsters._spellHP[slot];
			if (hp >= dmg + borrow) {
				hp -= dmg + borrow;
				writeString("\n");
				return true;
			}
		}

		killMonster(slot);
		return true;
	}

	case Data::SPELL_BLINK: {
		int x = (randByte() & 0xF) | 1;
		int y = (randByte() & 0xF) | 1;
		if (cellAt(x, y) != 0)
			break;

		sg._mapX = x;
		sg._mapY = y;
		writeString("\n");
		return true;
	}

	default:
		break;
	}

	playFX(Data::SFX_FAIL);
	writeString("-FAILED!\n");
	return true;
}

void DungeonLogic::spawnMonsters() {
	Data::MapMonsters &monsters = _G(map)._monsters;

	for (int slot = 31; slot >= 1; --slot) {
		if (monsters.isActive(slot))
			continue;

		// Each slot belongs to a single level, and picks an odd-numbered cell
		int level = slot >> 1;
		byte x = ((randByte() & 7) * 2) | 1;
		byte y = ((randByte() & 7) * 2) | 1;
		monsters._mapX[slot] = x;
		monsters._mapY[slot] = y;
		monsters._glyphTile[slot] = level;

		byte &cell = _G(dungeon).cell(level, x, y);
		if (cell != 0)
			continue;

		// Tougher monsters only turn up on deeper levels
		int r = randByte() & 7;
		if (r == 7 || r * 2 > level)
			continue;

		int type = r + 1;
		monsters._type[slot] = type;
		monsters._spellHP[slot] = (byte)((type * 2 + level) * 8) | 0x10;
		cell |= type;
	}
}

bool DungeonLogic::monsterAttacks(int slot) {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;
	int level = sg._dungeonLevel;

	if (randByte() < 0x40) {
		if (monsters._glyphTile[slot] != level)
			return true;

		switch (monsters._type[slot]) {
		case 2:
			if (sg._lightTurns != 0) {
				writeString("YOUR TORCH IS BLOWN OUT!\n");
				sg._lightTurns = 0;
			}
			return true;

		case 5:
			writeString("A GREMLIN STOLE SOME FOOD!\n");
			if (sg._food < 100) {
				playerDied();
				return false;
			}
			sg._food -= 100;
			return true;

		case 7:
			writeString("YOU FEEL A STRONG MAGIC!\n");
			sg._sleepTurns = randByte() & 7;
			return true;

		default:
			break;
		}
	}

	if (monsters._glyphTile[slot] != level)
		return true;

	playFX(Data::SFX_CANNON);
	byte roll = randByte();
	if ((roll & 0x80) != 0 || (roll & 7) < sg._readiedArmor)
		return true;

	showAttackTile(0, 0);
	int dmg = Data::bcdValue((byte)((randByte() & 0x77) + ((level * 4) & 0x77)));
	if (sg._hp < dmg) {
		sg._hp = 0;
		playerDied();
		return false;
	}

	sg._hp -= dmg;
	return true;
}

int DungeonLogic::tryMoveMonster(int slot, int x, int y) {
	Data::MapMonsters &monsters = _G(map)._monsters;
	int level = _G(savegame)._dungeonLevel;

	if (monsters._glyphTile[slot] != level)
		return 0;

	byte &dest = cellAt(x, y);
	if (dest == CELL_WALL || dest == 0xFF)
		return 0;
	if ((dest & 7) != 0)
		return 2;

	cellAt(monsters._mapX[slot], monsters._mapY[slot]) &= 0xF0;
	monsters._mapX[slot] = x;
	monsters._mapY[slot] = y;
	dest |= monsters._type[slot];
	return 1;
}

void DungeonLogic::updateCreatures() {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;

	spawnMonsters();

	for (int slot = 31; slot >= 1; --slot) {
		if (!monsters.isActive(slot))
			continue;

		int dx = signOf((int8)(byte)(sg._mapX - monsters._mapX[slot]));
		int dy = signOf((int8)(byte)(sg._mapY - monsters._mapY[slot]));

		// Badly hurt monsters run away
		if (monsters._spellHP[slot] < 0x0F) {
			dx = -dx;
			dy = -dy;
		}

		if (dx != 0) {
			byte x = monsters._mapX[slot] + dx, y = monsters._mapY[slot];
			if (x == sg._mapX && y == sg._mapY) {
				if (!monsterAttacks(slot))
					return;
				continue;
			}

			if (tryMoveMonster(slot, x, y) != 0)
				continue;
		}

		if (dy != 0) {
			byte x = monsters._mapX[slot], y = monsters._mapY[slot] + dy;
			if (x == sg._mapX && y == sg._mapY) {
				if (!monsterAttacks(slot))
					return;
				continue;
			}

			tryMoveMonster(slot, x, y);
		}
	}

	if (sg._lightTurns == 0) {
		writeString("IT'S DARK!\n");
	} else if (--sg._lightTurns == 0) {
		writeString("TORCH BURNED OUT!\n");
	}

	if (!sg.deductFood(16))
		playerDied();
}

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima
