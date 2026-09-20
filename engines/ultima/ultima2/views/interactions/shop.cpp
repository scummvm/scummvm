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

#include "ultima/ultima2/views/interactions/shop.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

Shop::Shop(Kind kind, const Common::String &name) : Interaction(name), _kind(kind) {
}

bool Shop::isValidChoice(int digit) const {
	switch (_kind) {
	case WEAPON:
		return digit >= 1 && digit <= 8;
	default:
		return digit >= 1 && digit <= 6;
	}
}

bool Shop::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	_state = CHOOSE;
	Data::Savegame &sg = _G(savegame);

	switch (_kind) {
	case WEAPON:
		writeString("  THE WEAPONS SHOPPE:\n1-DA, 2-MA, 3-AX, 4-BO,\n5-SW, 6-GR, 7-LI, 8-PH.\nYOUR INTEREST? ");
		break;
	case ARMOR:
		writeString("     THE ARMOUR SHOPPE:\n1-CLOTH, 2-LEATHER, 3-CHAIN,\n4-PLATE, 5-REFLECT, 6-POWER,\nYOUR INTEREST? ");
		break;
	case SPELL_CLERIC:
		writeString("WELCOME %s%s\n1-LIGHT, 2-L.D., 3-L.U.,\n4-PASS 5-SURFACE, 6-PRAYER.\nYOUR INTEREST? ",
			sg._sex == Data::SEX_FEMALE ? "SISTER " : "BROTHER ", sg._name);
		break;
	case SPELL_WIZARD:
		writeString("%s MUSERREF OLDUM!\n1-LIGHT, 2-L.D., 3-L.U.,\n4-M.M., 5-BLINK, 6-KILL,\nBIR IKI UC...? ", sg._name);
		break;
	}

	return Interaction::msgFocus(msg);
}

void Shop::finish(const char *message) {
	close();
	writeString("%s\n", message);
	_G(logic)->resumeTurn();
}

void Shop::chooseItem(int digit) {
	if (digit >= 0)
		writeString("%d\n", digit);

	if (!isValidChoice(digit)) {
		switch (_kind) {
		case WEAPON:
		case ARMOR:
			finish("THANKS FOR COMING BY...");
			break;
		case SPELL_CLERIC:
			finish("FOLLOW THE LIGHT, FRIEND!");
			break;
		case SPELL_WIZARD:
			finish("\nUGURLA OLSUN!");
			break;
		}
		return;
	}

	Data::Savegame &sg = _G(savegame);
	switch (_kind) {
	case WEAPON:
		_item = digit;
		_price = sg.computeItemPrice(_item);
		writeString("AH! YES! A %s\nFOR YOU ONLY %.4d\nHOW 'BOUT IT? ", Data::WEAPON_NAMES[_item], _price);
		break;
	case ARMOR:
		_item = digit;
		_price = sg.computeItemPrice(_item * 2);
		writeString("AH! YES! %s\nFOR YOU ONLY %.4d\nHOW 'BOUT IT? ", Data::ARMOR_NAMES[_item], _price);
		break;
	case SPELL_CLERIC:
	case SPELL_WIZARD:
		_item = (_kind == SPELL_WIZARD && digit >= 4) ? digit + 3 : digit;
		_price = sg.computeItemPrice(_item);
		writeString("FIVE %sS FOR %.4d\n%s ", Data::SPELL_NAMES[_item], _price,
			_kind == SPELL_CLERIC ? "YES, FRIEND?" : "LUTFEN EVET?");
		break;
	}

	_state = CONFIRM;
}

void Shop::confirm(char key) {
	Data::Savegame &sg = _G(savegame);
	bool yes = key == 'Y';
	close();

	switch (_kind) {
	case WEAPON:
	case ARMOR:
		if (key >= ' ')
			writeString("%c", key);
		writeString("\n");
		if (!yes) {
			writeString("OH, WELL.\n");
		} else if (_G(logic)->trySpendGold(_price)) {
			if (_kind == WEAPON)
				++sg._weaponOwned[_item];
			else
				++sg._armorOwned[_item];
			writeString("SOLD!\n");
		}
		break;

	case SPELL_CLERIC:
		if (!yes) {
			writeString("NO\nI'M SORRY, GOOD DAY.\n");
		} else {
			writeString("YES,\nI WILL TAKE 5!");
			if (_G(logic)->trySpendGold(_price)) {
				sg._spellCharges[_item] += 5;
				writeString("\n");
			}
		}
		break;

	case SPELL_WIZARD:
		if (!yes) {
			writeString("NO!\nUGURLA OLSUN!\n");
		} else {
			writeString("YES\n");
			if (_G(logic)->trySpendGold(_price)) {
				sg._spellCharges[_item] += 5;
				writeString("GULE GULE!\n");
			}
		}
		break;
	}

	_G(logic)->resumeTurn();
}

bool Shop::msgKeypress(const KeypressMessage &msg) {
	if (_state == CHOOSE) {
		// Any non-digit key leaves the shop, the same as an invalid choice
		chooseItem((msg.ascii >= '0' && msg.ascii <= '9') ? msg.ascii - '0' : -1);
	} else {
		confirm(toupper(msg.ascii));
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
