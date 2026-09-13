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

#include "ultima/ultima2/views/interactions/wear_armor.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

WearArmor::WearArmor() : Interaction("WearArmor") {
}

bool WearArmor::msgFocus(const FocusMessage &msg) {
	writeString("WEAR ARMOUR:\n1-CLOTH, 2-LEATHER, 3-CHAIN,\n4-PLATE, 5-REFLECT, 6-POWER,\nWHICH? ");
	return Interaction::msgFocus(msg);
}

bool WearArmor::msgKeypress(const KeypressMessage &msg) {
	if (msg.ascii < '0' || msg.ascii > '9')
		return true;

	int digit = msg.ascii - '0';
	int armorIdx = (digit == 7) ? Data::ARMOR_SKIN : digit;
	Data::Savegame &sg = _G(savegame);

	if (armorIdx >= Data::ARMOR_COUNT) {
		writeString("%d\nNOT OWNED!\n", digit);
	} else if (armorIdx != Data::ARMOR_SKIN && sg._armorOwned[armorIdx] == 0) {
		writeString("%d\n%s NOT OWNED!\n", digit, Data::ARMOR_NAMES[armorIdx]);
	} else if (armorIdx * 8 >= sg._strength) {
		writeString("%d\n<-THOU ART NOT\nSTRONG ENOUGH TO WEAR!\n", digit);
	} else {
		sg._readiedArmor = (Data::ArmorType)armorIdx;
		writeString("%d\n%s READY.\n", digit, Data::ARMOR_NAMES[armorIdx]);
	}

	close();
	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
