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

#include "mm/mm1/views/locations/blacksmith.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

Blacksmith::Blacksmith() : Location("Blacksmith") {
}

bool Blacksmith::msgFocus(const FocusMessage &msg) {
	send("View", GameMessage("LOCATION", LOC_BLACKSMITH));
	changeCharacter(0);

	return true;
}

void Blacksmith::draw() {
	Location::draw();

	writeString(22, 1, STRING["dialogs.blacksmith.a"]);
	writeString(22, 2, STRING["dialogs.blacksmith.b"]);
	writeString(22, 3, STRING["dialogs.blacksmith.c"]);
	writeString(22, 4, STRING["dialogs.blacksmith.d"]);
}

bool Blacksmith::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_1:
	case Common::KEYCODE_2:
	case Common::KEYCODE_3:
	case Common::KEYCODE_4:
	case Common::KEYCODE_5:
	case Common::KEYCODE_6:
		changeCharacter(msg.keycode - Common::KEYCODE_1);
		break;
	case Common::KEYCODE_a:
		_buyWeapons.addView();
		break;
	case Common::KEYCODE_b:
		_buyArmor.addView();
		break;
	case Common::KEYCODE_c:
		_buyMisc.addView();
		break;
	case Common::KEYCODE_d:
		_sellItem.addView();
		break;
	default:
		break;
	}

	return true;
}

bool Blacksmith::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		leave();
		return true;
	}

	return false;
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
