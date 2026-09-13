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

#include "ultima/ultima2/logic/city_castle_logic.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

void CityCastleLogic::exitToOverworld() {
	Data::Savegame &sg = _G(savegame);
	sg._mapX = sg._overworldReturnX;
	sg._mapY = sg._overworldReturnY;
	sg._mapNum2 = 0;
	_G(map).load(sg._mapNum1, sg._mapNum2);
}

bool CityCastleLogic::move(Data::Direction dir) {
	Data::Savegame &sg = _G(savegame);

	int newX = sg._mapX, newY = sg._mapY;
	const char *dirName;
	switch (dir) {
	case Data::DIR_UP:    --newY; dirName = "NORTH"; break;
	case Data::DIR_DOWN:  ++newY; dirName = "SOUTH"; break;
	case Data::DIR_LEFT:  --newX; dirName = "WEST"; break;
	case Data::DIR_RIGHT: ++newX; dirName = "EAST"; break;
	default: return true;
	}

	writeString("%s", dirName);

	if (sg._legParalysisTurns > 0) {
		writeString("--PARALIZED!--INVALID MOVE!\n");
		return true;
	}

	if (newX < 0 || newX >= Data::MAP_WIDTH || newY < 0 || newY >= Data::MAP_HEIGHT) {
		writeString("\n");
		exitToOverworld();
		return false;
	}

	Data::TileId destTile = _G(map).tileAt(newX, newY);

	if (destTile == Data::TILE_SWAMP) {
		if (!sg.deductHP(5)) {
			playerDied();
			return false;
		}
	} else if (destTile == Data::TILE_FORCEFIELD) {
		if (sg._items[Data::ITEM_RING] != 0) {
			writeString("\nRING PROTECTS FROM FIELD!\n");
		} else {
			writeString("\nFIELD CAUSES 1000 DAMAGE!\n");
			if (!sg.deductHP(1000)) {
				playerDied();
				return false;
			}
		}
	}

	if (!sg.deductFood(25)) {
		playerDied();
		return false;
	}

	if ((!_G(intangible) && !isWalkable(destTile)) || findTargetMonster(newX, newY) >= 0) {
		writeString("--INVALID MOVE!\n");
		return true;
	}

	writeString("\n");
	sg._mapX = newX;
	sg._mapY = newY;
	return true;
}

void CityCastleLogic::updateCreatures() {
	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;
	byte damageAccumulator = 0;
	int engagedCount = 0, hitCount = 0;

	for (int slot = 31; slot >= 1; --slot) {
		if (!monsters.isActive(slot))
			continue;

		int8 flag = (int8)monsters._offerFlag[slot];
		if (flag <= 0)
			continue; // at rest (0), or a shop/quest NPC (negative/high-bit) - never moves

		int sdx, sdy;
		if (flag <= 2) {
			int dx = sg._mapX - monsters._mapX[slot];
			int dy = sg._mapY - monsters._mapY[slot];
			sdx = signByte(dx * 4);
			sdy = signByte(dy * 4);
		} else {
			// Wander: 75% chance to pick a fresh random direction, else
			// keep going the way it was already headed (kept in _tempX/Y)
			if (randByte() < 0xC0 || (monsters._tempX[slot] == 0 && monsters._tempY[slot] == 0)) {
				static const int8 DX[4] = { 0, 0, -1, 1 };
				static const int8 DY[4] = { -1, 1, 0, 0 };
				int r = randByte() & 3;
				sdx = DX[r];
				sdy = DY[r];
				monsters._tempX[slot] = (byte)sdx;
				monsters._tempY[slot] = (byte)sdy;
			} else {
				sdx = (int8)monsters._tempX[slot];
				sdy = (int8)monsters._tempY[slot];
			}
		}

		int newX = (monsters._mapX[slot] + sdx + Data::MAP_WIDTH) % Data::MAP_WIDTH;
		int newY = (monsters._mapY[slot] + sdy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;

		if (flag <= 2 && newX == sg._mapX && newY == sg._mapY) {
			++engagedCount;

			int roll = randByte();
			if (roll < 0x80 && (roll & 7) >= sg._readiedArmor) {
				++hitCount;
				int contribution = (monsters._spellHP[slot] >> 2) + 1 + damageAccumulator;
				damageAccumulator = (byte)contribution;
			}
		} else if (newX != sg._mapX || newY != sg._mapY) {
			if (monsters._spellHP[slot] < 16)
				++monsters._spellHP[slot];

			if (!tryMoveMonster(slot, sdx, sdy) && !tryMoveMonster(slot, 0, sdy))
				tryMoveMonster(slot, sdx, 0);
		}
	}

	for (int i = 0; i < engagedCount; ++i) {
		playFX(1);
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

	// Villages/towns/castles don't spawn new monsters
}

bool CityCastleLogic::steal(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("STEAL--");
		_directionPurpose = DirectionPurpose::STEAL;
		g_engine->addView("Direction");
		return false;
	}

	Data::Savegame &sg = _G(savegame);
	int dx = 0, dy = 0;
	switch (dir) {
	case Data::DIR_UP: dy = -1; break;
	case Data::DIR_DOWN: dy = 1; break;
	case Data::DIR_LEFT: dx = -1; break;
	case Data::DIR_RIGHT: dx = 1; break;
	default: break;
	}

	int x1 = sg._mapX + dx, y1 = sg._mapY + dy;
	bool counterFound = x1 >= 0 && x1 < Data::MAP_WIDTH && y1 >= 0 && y1 < Data::MAP_HEIGHT &&
		(_G(map).tileAt(x1, y1) >= Data::TILE_A || _G(map).tileAt(x1, y1) == Data::TILE_EMPTY_COUNTER);

	int x2 = sg._mapX + dx * 2, y2 = sg._mapY + dy * 2;
	bool foundShopkeep = counterFound && x2 >= 0 && x2 < Data::MAP_WIDTH && y2 >= 0 && y2 < Data::MAP_HEIGHT &&
		_G(map).tileAt(x2, y2) == Data::TILE_SHOPKEEP;

	auto noLuck = [this]() {
		writeString("NO LUCK!\n");
		if ((randByte() & 7) == 0)
			alertTownGuards();
	};

	if (!foundShopkeep) {
		noLuck();
		return true;
	}
	if (sg._class != Data::CLASS_THIEF && (randByte() & 0x80) != 0) {
		noLuck();
		return true;
	}
	if ((randByte() & 0x80) != 0) {
		noLuck();
		return true;
	}

	Data::MapMonsters &monsters = _G(map)._monsters;
	if (monsters._offerFlag[0] == 1) {
		noLuck();
		return true;
	}

	switch (sg._mapNum2) {
	case 1: // Village
		if (sg._mapY < 32 && sg._mapX >= 32) {
			writeString("STEAL FOOD!\n");
			++sg._food;
		} else {
			noLuck();
		}
		break;
	case 2: // Town
		if (sg._mapY >= 32) {
			noLuck();
		} else if (sg._mapX >= 32) {
			writeString("STEAL ARMOUR!\n");
			++sg._armorOwned[1 + (randByte() & 3)];
		} else {
			writeString("STEAL WEAPONS!\n");
			++sg._weaponOwned[1 + (randByte() & 7)];
		}
		break;
	default: // Castle - never works
		noLuck();
		break;
	}

	return true;
}

bool CityCastleLogic::unlock(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		_directionPurpose = DirectionPurpose::UNLOCK;
		g_engine->addView("Direction");
		return false;
	}

	Data::Savegame &sg = _G(savegame);
	int dx = 0, dy = 0;
	switch (dir) {
	case Data::DIR_UP: dy = -1; break;
	case Data::DIR_DOWN: dy = 1; break;
	case Data::DIR_LEFT: dx = -1; break;
	case Data::DIR_RIGHT: dx = 1; break;
	default:
		writeString("NO DOOR THERE!\n");
		return true;
	}

	int x = sg._mapX + dx, y = sg._mapY + dy;
	if (x < 0 || x >= Data::MAP_WIDTH || y < 0 || y >= Data::MAP_HEIGHT ||
			_G(map).tileAt(x, y) != Data::TILE_I_DOOR) {
		writeString("NO DOOR THERE!\n");
		return true;
	}

	if (sg._keys == 0) {
		writeString("NO KEYS THAT FIT!\n");
		return true;
	}

	--sg._keys;
	// The door tile is replaced by whatever tile the player is currently
	// standing on - matches the original exactly, though it's an odd way
	// to clear a door; worth confirming visually during playtest
	_G(map)._tiles[y][x] = _G(map).tileAt(sg._mapX, sg._mapY);
	writeString("UNLOCKED!\n");
	return true;
}

bool CityCastleLogic::offer(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		_directionPurpose = DirectionPurpose::OFFER;
		g_engine->addView("Direction");
		return false;
	}

	Data::Savegame &sg = _G(savegame);
	int dx = 0, dy = 0;
	switch (dir) {
	case Data::DIR_UP: dy = -1; break;
	case Data::DIR_DOWN: dy = 1; break;
	case Data::DIR_LEFT: dx = -1; break;
	case Data::DIR_RIGHT: dx = 1; break;
	default: break;
	}

	int x = sg._mapX + dx, y = sg._mapY + dy;
	int slot = (x >= 0 && x < Data::MAP_WIDTH && y >= 0 && y < Data::MAP_HEIGHT) ?
		findTargetMonster(x, y) : -1;

	if (slot < 0) {
		writeString("OFFER TO WHOM?\n");
		return true;
	}

	_offerTargetSlot = slot;
	writeString("HOW MUCH (*100)? ");
	g_engine->addView("OfferGold");
	return false;
}

void CityCastleLogic::completeOffer(int goldHundreds) {
	Data::Savegame &sg = _G(savegame);
	if (!trySpendGold(goldHundreds * 100)) {
		resumeTurn();
		return;
	}

	Data::MapMonsters &monsters = _G(map)._monsters;
	byte flag = monsters.isActive(_offerTargetSlot) ? monsters._offerFlag[_offerTargetSlot] : 0;
	bool wrongMap = !(sg._mapNum2 == 2 && sg._mapNum1 == 3);

	if ((int8)flag >= 0 || wrongMap) {
		writeString("THANK YOU VERY MUCH!\n");
		resumeTurn();
		return;
	}

	switch (flag) {
	case 0x81:
		if (goldHundreds < 5) {
			writeString("THANK YOU VERY MUCH!\n");
		} else {
			writeString("ENILNO IS YOURS!\n");
			sg._enilnoOwned = true;
		}
		break;

	case 0x82:
		if (goldHundreds < 5) {
			writeString("THANK YOU VERY MUCH!\n");
		} else if (!sg._ringQuestFlag) {
			writeString("EARN THE RING!\n");
		} else {
			writeString("THE RING IS YOURS!\n");
			++sg._items[Data::ITEM_RING];
		}
		break;

	case 0x83: {
		int idx = 1 + (randByte() & 7);
		++sg._offerRewardItems[idx];
		writeString("HERE TAKE THIS!\n");
		break;
	}

	default:
		if ((randByte() & 7) >= 6) {
			writeString("THANK YOU VERY MUCH!\n");
		} else {
			int16 *attrs[6] = {
				&sg._strength, &sg._agility, &sg._stamina,
				&sg._charisma, &sg._wisdom, &sg._intelligence
			};
			*attrs[randByte() % 6] += 4 * goldHundreds;
			writeString("ALAKAZAM!\n");
		}
		break;
	}

	resumeTurn();
}

bool CityCastleLogic::tithe() {
	Data::Savegame &sg = _G(savegame);
	if (sg._class != Data::CLASS_CLERIC) {
		writeString(" YOU ARE NOT A CLERIC!\n");
		return true;
	}

	writeString("WELCOME %s\n", sg._name);
	writeString("FIRST MY 50 G.P. TRIBUTE!\n");
	if (!trySpendGold(50))
		return true;

	int heal = (sg._hp < 50) ? 3 : (sg._hp < 75) ? 2 : (sg._hp < 99) ? 1 : 0;
	sg._hp += heal;
	writeString("AND FOR IT I RAISE THEE %d\n", heal);
	return true;
}

bool CityCastleLogic::openShopForCurrentQuadrant() {
	Data::Savegame &sg = _G(savegame);
	bool top = sg._mapY < 32;
	bool left = sg._mapX < 32;
	bool isTown = sg._mapNum2 == 2;

	if (top && left) {
		if (isTown) {
			g_engine->addView("WeaponShop");
			return false;
		}
		writeString("A HOODED FIGURE OFFERS A CLUE FOR GOLD...\n");
		return true;
	}

	if (!top && left) {
		if (isTown) {
			writeString("WELCOME TO THE PUB!\n");
			return true;
		}
		g_engine->addView("WizardSpellShop");
		return false;
	}

	if (top) { // top-right
		if (isTown) {
			g_engine->addView("ArmorShop");
			return false;
		}
		writeString("PER 100, WANT ONE? ");
		if (!trySpendGold(sg.computeItemPrice(3)))
			return true;
		++sg._food;
		writeString("\nOK!\n");
		return true;
	}

	// bottom-right
	if (isTown) {
		writeString("RIDE SWIFTLY!\n");
		trySpendGold(sg.computeItemPrice(4));
		return true;
	}
	if (sg._class != Data::CLASS_CLERIC) {
		writeString(" YOU ARE NOT A CLERIC!\n");
		return true;
	}
	g_engine->addView("ClericSpellShop");
	return false;
}

bool CityCastleLogic::transact(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("TRANSACT-");
		_directionPurpose = DirectionPurpose::TRANSACT;
		g_engine->addView("Direction");
		return false;
	}

	Data::Savegame &sg = _G(savegame);
	int dx = 0, dy = 0;
	const char *dirName;
	switch (dir) {
	case Data::DIR_UP:    dy = -1; dirName = "NORTH"; break;
	case Data::DIR_DOWN:  dy = 1;  dirName = "SOUTH"; break;
	case Data::DIR_LEFT:  dx = -1; dirName = "WEST"; break;
	case Data::DIR_RIGHT: dx = 1;  dirName = "EAST"; break;
	default: dirName = ""; break;
	}
	writeString("%s\n", dirName);

	int x1 = sg._mapX + dx, y1 = sg._mapY + dy;
	int slot = (x1 >= 0 && x1 < Data::MAP_WIDTH && y1 >= 0 && y1 < Data::MAP_HEIGHT) ?
		findTargetMonster(x1, y1) : -1;

	if (slot > 0) {
		Data::MapMonsters &monsters = _G(map)._monsters;
		if ((int8)monsters._offerFlag[slot] < 0)
			return openShopForCurrentQuadrant();

		switch (monsters.tileType(slot)) {
		case Data::TILE_TITHE_ALTAR:
			return tithe();
		case Data::TILE_GUARD:
			writeString("A GUARD SAYS:\nPAY YOUR TAXES!\n");
			return true;
		case Data::TILE_JESTER:
			writeString("A JESTER SINGS:\nHO HO HO!\n");
			return true;
		case Data::TILE_SHOPKEEP:
			writeString("A MERCHANT SAYS:\nWILL YOU BUY MY APPLES?\n");
			return true;
		case Data::TILE_FIGHTER:
			writeString("A FIGHTER SAYS:\nUGH, ME TOUGH!\n");
			return true;
		case Data::TILE_CLERIC:
			writeString("A CLERIC SAYS:\nBELIEVE!\n");
			return true;
		case Data::TILE_MAGE:
			writeString("A WIZARD SAYS:\nHEX-E-POO-HEX-ON-YOU!\n");
			return true;
		case Data::TILE_THIEF:
			writeString("A THIEF SAYS:\nPSST, WANNA BUY A WATCH?\n");
			return true;
		default:
			writeString("FUNNY, NO RESPONSE!\n");
			return true;
		}
	}

	// No monster in that direction - counter-tile probe fallback
	Data::TileId t1 = (x1 >= 0 && x1 < Data::MAP_WIDTH && y1 >= 0 && y1 < Data::MAP_HEIGHT) ?
		_G(map).tileAt(x1, y1) : Data::TILE_WATER;
	if (t1 < Data::TILE_A && t1 != Data::TILE_EMPTY_COUNTER) {
		writeString("FUNNY, NO RESPONSE!\n");
		return true;
	}

	int x2 = sg._mapX + dx * 2, y2 = sg._mapY + dy * 2;
	Data::TileId t2 = (x2 >= 0 && x2 < Data::MAP_WIDTH && y2 >= 0 && y2 < Data::MAP_HEIGHT) ?
		_G(map).tileAt(x2, y2) : Data::TILE_WATER;
	if (t2 != Data::TILE_SHOPKEEP) {
		writeString("FUNNY, NO RESPONSE!\n");
		return true;
	}

	return openShopForCurrentQuadrant();
}

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima
