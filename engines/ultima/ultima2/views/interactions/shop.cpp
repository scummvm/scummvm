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

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

Shop::Shop(Kind kind, const Common::String &name) : Interaction(name), _kind(kind) {
}

int Shop::priceIndexForDigit(int digit) const {
	switch (_kind) {
	case ARMOR:
		return digit * 2;
	case SPELL_WIZARD:
		return (digit >= 4) ? digit + 3 : digit;
	default:
		return digit;
	}
}

bool Shop::msgFocus(const FocusMessage &msg) {
	switch (_kind) {
	case WEAPON:
		writeString("WEAPON SHOPPE\n1-DA,2-MA,3-AX,4-BO,5-SW,6-GR,7-LI,8-PH\nWHICH? ");
		break;
	case ARMOR:
		writeString("ARMOUR SHOPPE\n1-CLOTH,2-LEATHER,3-CHAIN,\n4-PLATE,5-REFLECT,6-POWER\nWHICH? ");
		break;
	case SPELL_CLERIC:
		writeString("MAGIC SHOPPE\n1-LIGHT,2-L.D.,3-L.U.,\n4-PASS,5-SURFACE,6-PRAYER\nWHICH? ");
		break;
	case SPELL_WIZARD:
		writeString("MAGIC SHOPPE\n1-LIGHT,2-L.D.,3-L.U.,\n4-M.M.,5-BLINK,6-KILL\nWHICH? ");
		break;
	}

	return Interaction::msgFocus(msg);
}

bool Shop::msgKeypress(const KeypressMessage &msg) {
	if (msg.ascii < '0' || msg.ascii > '9')
		return true;

	int digit = msg.ascii - '0';
	close();

	if (digit != 0 && digit <= 6) {
		Data::Savegame &sg = _G(savegame);
		int price = sg.computeItemPrice(priceIndexForDigit(digit));

		if (_G(logic)->trySpendGold(price)) {
			switch (_kind) {
			case WEAPON:
				++sg._weaponOwned[digit];
				writeString("%s READY.\n", Data::WEAPON_NAMES[digit]);
				break;
			case ARMOR:
				++sg._armorOwned[digit];
				writeString("%s READY.\n", Data::ARMOR_NAMES[digit]);
				break;
			case SPELL_CLERIC:
				sg._spellCharges[digit] += 5;
				writeString("%s CHARGED.\n", Data::SPELL_NAMES[digit]);
				break;
			case SPELL_WIZARD: {
				int spellIdx = (digit >= 4) ? digit + 3 : digit;
				sg._spellCharges[spellIdx] += 5;
				writeString("%s CHARGED.\n", Data::SPELL_NAMES[spellIdx]);
				break;
			}
			}
		}
	}

	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
