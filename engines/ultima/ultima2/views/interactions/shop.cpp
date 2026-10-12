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
		writeString("  The Weapons Shoppe:\n1-Da, 2-Ma, 3-Ax, 4-Bo,\n5-Sw, 6-Gr, 7-Li, 8-Ph.\nYour interest? ");
		break;
	case ARMOR:
		writeString("     The Armour Shoppe:\n1-Cloth, 2-Leather, 3-Chain,\n4-Plate, 5-Reflect, 6-Power,\nYour interest? ");
		break;
	case SPELL_CLERIC:
		writeString("Welcome %s%s\n1-Light, 2-L.D., 3-L.U.,\n4-Pass 5-Surface, 6-Prayer.\nYour interest? ",
			sg._sex == Data::SEX_FEMALE ? "Sister " : "Brother ", sg._name);
		break;
	case SPELL_WIZARD:
		writeString("%s Muserref Oldum!\n1-Light, 2-L.D., 3-L.U.,\n4-M.M., 5-Blink, 6-Kill,\nBir iki uc...? ", sg._name);
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
			finish("Thanks for coming by...");
			break;
		case SPELL_CLERIC:
			finish("Follow the Light, friend!");
			break;
		case SPELL_WIZARD:
			finish("\nUgurla olsun!");
			break;
		}
		return;
	}

	Data::Savegame &sg = _G(savegame);
	switch (_kind) {
	case WEAPON:
		_item = digit;
		_price = sg.computeItemPrice(_item);
		writeString("Ah! Yes! A %s\nFor you only %.4d\nHow 'bout it? ", Data::WEAPON_NAMES[_item], _price);
		break;
	case ARMOR:
		_item = digit;
		_price = sg.computeItemPrice(_item * 2);
		writeString("Ah! Yes! %s\nFor you only %.4d\nHow 'bout it? ", Data::ARMOR_NAMES[_item], _price);
		break;
	case SPELL_CLERIC:
	case SPELL_WIZARD:
		_item = (_kind == SPELL_WIZARD && digit >= 4) ? digit + 3 : digit;
		_price = sg.computeItemPrice(_item);
		writeString("Five %ss for %.4d\n%s ", Data::SPELL_NAMES[_item], _price,
			_kind == SPELL_CLERIC ? "Yes, friend?" : "Lutfen evet?");
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
			writeString("Oh, well.\n");
		} else if (_G(logic)->trySpendGold(_price)) {
			if (_kind == WEAPON)
				sg._weaponOwned[_item] = MIN(sg._weaponOwned[_item] + 1, Data::MAX_BCD_BYTE);
			else
				sg._armorOwned[_item] = MIN(sg._armorOwned[_item] + 1, Data::MAX_BCD_BYTE);
			writeString("Sold!\n");
		}
		break;

	case SPELL_CLERIC:
		if (!yes) {
			writeString("No\nI'm sorry, good day.\n");
		} else {
			writeString("Yes,\nI will take 5!");
			if (_G(logic)->trySpendGold(_price)) {
				sg._spellCharges[_item] = MIN(sg._spellCharges[_item] + 5, Data::MAX_BCD_BYTE);
				writeString("\n");
			}
		}
		break;

	case SPELL_WIZARD:
		if (!yes) {
			writeString("No!\nUgurla olsun!\n");
		} else {
			writeString("Yes\n");
			if (_G(logic)->trySpendGold(_price)) {
				sg._spellCharges[_item] = MIN(sg._spellCharges[_item] + 5, Data::MAX_BCD_BYTE);
				writeString("Gule gule!\n");
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
