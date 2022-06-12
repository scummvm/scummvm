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

#include "mm/mm1/views/businesses/training.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Businesses {

#define MAX_LEVEL 200

Training::Training() : Business("Training") {
}

bool Training::msgFocus(const FocusMessage &msg) {
	g_events->msgBusiness(BusinessMessage(LOC_TRAINING));
	changeCharacter(0);

	return true;
}

bool Training::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_ESCAPE:
		leave();
		break;
	case Common::KEYCODE_1:
	case Common::KEYCODE_2:
	case Common::KEYCODE_3:
	case Common::KEYCODE_4:
	case Common::KEYCODE_5:
	case Common::KEYCODE_6:
		changeCharacter(msg.keycode - Common::KEYCODE_1);
		break;
	default:
		break;
	}

	return true;
}

void Training::changeCharacter(uint index) {
	Maps::Map &map = *g_maps->_currentMap;
	int i;

	if (index >= g_globals->_party.size())
		return;
	Business::changeCharacter(index);
}

void Training::draw() {
	Character &c = *g_globals->_currCharacter;
	Business::draw();

	writeString(18, 1, STRING["dialogs.training.for_level"]);
	writeNumber(c._nextLevel);

	if (c._nextLevel >= MAX_LEVEL) {
		writeString(24, 3, STRING["dialogs.training.no_way"]);
	}
}

} // namespace Businesses
} // namespace Views
} // namespace MM1
} // namespace MM
