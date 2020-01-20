/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima1/widgets/merchant_armor.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

EMPTY_MESSAGE_MAP(MerchantArmor, Merchant);

void MerchantArmor::get() {
	Maps::MapCastle *map = dynamic_cast<Maps::MapCastle *>(_map);
	assert(map);
	if (map->_getCounter > 0) {
		--map->_getCounter;
		findArmor(false);
	} else {
		noKingsPermission();
	}
}

void MerchantArmor::steal() {
	findArmor(true);
}

void MerchantArmor::findArmor(bool checkStealing) {
	Shared::Character &c = *_game->_party;

	if (!checkStealing || !checkCuaghtStealing()) {
		uint armorNum = _game->getRandomNumber(1, 5);
		Common::String armorStr = _game->_res->ARMOR_NAMES[armorNum];
		c._armor[armorNum]->incrQuantity();

		if (armorNum == 5)
			armorStr = Common::String::format("%s %s", _game->_res->A, armorStr.c_str());

		addInfoMsg("");
		addInfoMsg(Common::String::format(_game->_res->FIND, armorStr.c_str()));
	}
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
