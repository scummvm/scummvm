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

#include "mm/mm1/game/equip_remove.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {

bool EquipRemove::equipItem(int index, Common::Point &textPos, Common::String &equipError) {
	Character &c = *g_globals->_currCharacter;
	uint itemId = c._backpack[index]._id;
	uint charges = c._backpack[index]._charges;

	int classBit = 0;
	textPos.x = 0;

	switch (c._class) {
	case KNIGHT:
		classBit = KNIGHT_BIT;
		break;
	case PALADIN:
		classBit = PALADIN_BIT;
		break;
	case ARCHER:
		classBit = ARCHER_BIT;
		break;
	case CLERIC:
		classBit = CLERIC_BIT;
		break;
	case SORCERER:
		classBit = SORCERER_BIT;
		break;
	case ROBBER:
		classBit = ROBBER_BIT;
		break;
	default:
		equipError = STRING["dialogs.character.wrong_class"];
		break;
	}

	g_globals->_items.getItem(itemId);
	const Item &item = g_globals->_currItem;

	if (equipError.empty() && (item._disablements & classBit))
		equipError = STRING["dialogs.character.wrong_class"];
	if (equipError.empty()) {
		int alignBit = 0;
		switch (c._alignment) {
		case GOOD:
			alignBit = GOOD_BIT;
			break;
		case NEUTRAL:
			alignBit = NEUTRAL_BIT;
			break;
		case EVIL:
			alignBit = EVIL_BIT;
			break;
		default:
			equipError = STRING["dialogs.character.wrong_alignment"];
			break;
		}

		if ((item._disablements & alignBit) && alignBit != NEUTRAL_BIT)
			equipError = STRING["dialogs.character.wrong_alignment"];
	}

	if (equipError.empty()) {
		if (item._constBonus_id == IS_EQUIPPABLE) {
			equipError = STRING["dialogs.character.not_equipped"];
			textPos.x = 10;
		}
	}

	if (equipError.empty()) {
		if (isWeapon(itemId)) {
			if (c._equipped.hasWeapon() || c._equipped.hasTwoHanded())
				equipError = STRING["dialogs.character.have_weapon"];
		} else if (isMissile(itemId)) {
			if (c._equipped.hasMissile()) {
				equipError = STRING["dialogs.character.have_missile"];
				textPos.x = 3;
			}
		} else if (isTwoHanded(itemId)) {
			if (c._equipped.hasShield()) {
				equipError = STRING["dialogs.character.cannot_with_shield"];
				textPos.x = 7;
			} else if (c._equipped.hasWeapon()) {
				equipError = STRING["dialogs.character.have_weapon"];

			}
		} else if (isArmor(itemId)) {
			if (c._equipped.hasArmor()) {
				equipError = STRING["dialogs.character.have_armor"];
				textPos.x = 5;
			}
		} else if (isShield(itemId)) {
			if (c._equipped.hasTwoHanded()) {
				equipError = STRING["dialogs.character.cannot_two_handed"];
				textPos.x = 1;
			}
		} else if (itemId == 255) {
			equipError = STRING["dialogs.character.not_equipped"];
			textPos.x = 10;
		}
	}

	if (equipError.empty() && c._equipped.full()) {
		equipError = STRING["dialogs.character.full"];
		textPos.x = 14;
	}

	if (equipError.empty()) {
		// All checks passed, can equip item
		c._backpack.removeAt(index);
		uint freeIndex = c._equipped.add(itemId, charges);

		if (item._constBonus_id != NO_EQUIP_BONUS) {
			if (item._constBonus_id == IS_EQUIPPABLE) {
				equipError = STRING["dialogs.character.not_equipped"];
				textPos.x = 10;
			} else if (item._constBonus_id == EQUIP_CURSED) {
				c._equipped[freeIndex]._charges += item._constBonus_value;
			}
		}
	}

	if (!equipError.empty())
		return false;

	//add const equip bonus to character parameters
	applyEquipBonus(item._constBonus_id, item._constBonus_value);

	switch (getItemCategory(itemId)) {
	case ITEMCAT_WEAPON:
	case ITEMCAT_TWO_HANDED:
		c._physicalAttr._base = item._damage;
		c._physicalAttr._current = item._AC_Dmg;
		break;
	case ITEMCAT_MISSILE:
		c._missileAttr._base = item._damage;
		c._missileAttr._current = item._AC_Dmg;
		break;
	case ITEMCAT_ARMOR:
	case ITEMCAT_SHIELD:
		c._ac._base += item._AC_Dmg;
		break;
	default:
		break;
	}

	c.updateResistances();
	c.updateAttributes();
	c.updateAC();
	return true;
}

bool EquipRemove::removeItem(int index, Common::Point &textPos, Common::String &removeError) {
	Character &c = *g_globals->_currCharacter;
	uint itemId = c._equipped[index]._id;
	uint charges = c._equipped[index]._charges;

	g_globals->_items.getItem(itemId);
	const Item &item = g_globals->_currItem;
	if (item._constBonus_id == EQUIP_CURSED) {
		removeError = STRING["dialogs.character.cursed"];
		textPos.x = 13;
	} else if (c._backpack.full()) {
		removeError = STRING["dialogs.character.full"];
		textPos.x = 14;
	}

	if (!removeError.empty())
		return false;

	// Shift item to backpack
	c._equipped.removeAt(index);
	c._backpack.add(itemId, charges);

	if (item._constBonus_value) {
		// TODO: _equipMode is used as a character offset. Need to
		// find an example that calls it so I know what area of
		// the character updates are being done to
		//error("TODO: item flag in remove item");

		//subtract const equip bonus from character parameters
		applyEquipBonus(item._constBonus_id, -item._constBonus_value);
	}

	switch (getItemCategory(itemId)) {
	case ITEMCAT_WEAPON:
	case ITEMCAT_TWO_HANDED:
		c._physicalAttr.clear();
		break;
	case ITEMCAT_MISSILE:
		c._missileAttr.clear();
		break;
	case ITEMCAT_ARMOR:
	case ITEMCAT_SHIELD:
		c._ac._base = MAX((int)c._ac._base - (int)item._AC_Dmg, 0);
		break;
	default:
		break;
	}

	return true;
}

void EquipRemove::applyEquipBonus(int id, int value){
	if ((id<2)||(id>=0xff)) return;
	Character &c = *g_globals->_currCharacter;

	// TODO: check strange cases (decimal id numbers): 16, 19
	switch (id) {
		//case 16: c.sex = NONE; break;	//UNOBTAINIUM
		// case 19: c._luck._race = NONE; break;  //JADE AMULET
		case 21: c._intelligence._base += value; break;
		case 23: c._might._base += value; break;
		case 25: c._personality._base += value; break;
		case 29: c._speed._base += value; break;
		case 31: c._accuracy._base += value; break;
		case 33: c._luck._base += value; break;
		case 37: c._age += value; break;
		case 60: c._ac._base += value; break;
		case 88: c._resistances._s._magic._base += value; break; 
		case 90: c._resistances._s._fire._base += value; break;
		case 92: c._resistances._s._cold._base += value; break;
		case 94: c._resistances._s._electricity._base += value; break;
		case 96: c._resistances._s._acid._base += value; break;
		case 98: c._resistances._s._fear._base += value; break;
		case 100: c._resistances._s._poison._base += value; break;
		case 102: c._resistances._s._psychic._base += value; break; //resistance to sleep
		case 108: c._trapCtr += value; break;
	}
}


} // namespace Game
} // namespace MM1
} // namespace MM
