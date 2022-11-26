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

#include "mm/mm1/views/locations/blacksmith_buy_armor.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

BlacksmithBuyArmor::BlacksmithBuyArmor() :
		BlacksmithSubview("BlacksmithBuyArmor") {
	_modeString = STRING["dialogs.location.buy"];
}

bool BlacksmithBuyArmor::msgFocus(const FocusMessage &msg) {
	int townNum = g_maps->_currentMap->dataByte(Maps::MAP_ID);
	if (townNum < 1 || townNum >= 6)
		townNum = 1;
	_items = ARMOR[townNum - 1];

	return true;
}

void BlacksmithBuyArmor::draw() {
	Location::draw();

	writeString(23, 0, STRING["dialogs.blacksmith.armor"]);
	drawItems();
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
