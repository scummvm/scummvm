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

#include "ultima/ultima2/views/interactions/ready_weapon.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

ReadyWeapon::ReadyWeapon() : Interaction("ReadyWeapon") {
}

bool ReadyWeapon::msgFocus(const FocusMessage &msg) {
	writeString("READY WEAPON:\n1-DA, 2-MA, 3-AX, 4-BO,\n5-SW, 6-GR, 7-LI, 8-PH.\n9-QU, WHICH? ");
	return Interaction::msgFocus(msg);
}

bool ReadyWeapon::msgKeypress(const KeypressMessage &msg) {
	if (msg.ascii < '0' || msg.ascii > '9')
		return true;

	int digit = msg.ascii - '0';
	Data::Savegame &sg = _G(savegame);

	if (digit != Data::WEAPON_HANDS && sg._weaponOwned[digit] == 0) {
		writeString("%d\n%s NOT OWNED!\n", digit, Data::WEAPON_NAMES[digit]);
	} else if (digit * 8 >= sg._agility) {
		writeString("%d\n<-THOU ART NOT AGILE ENOUGH TO WIELD!\n", digit);
	} else {
		sg._readiedWeapon = (Data::WeaponType)digit;
		writeString("%d\n%s READY.\n", digit, Data::WEAPON_NAMES[digit]);
	}

	close();
	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
