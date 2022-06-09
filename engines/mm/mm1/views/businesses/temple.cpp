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

#include "mm/mm1/views/businesses/temple.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Businesses {

Temple::Temple() : Business("Temple") {
	_modeString = STRING["dialogs.business.gather"];
}

bool Temple::msgFocus(const FocusMessage &msg) {
	g_events->msgBusiness(BusinessMessage(2));
	g_globals->_currCharacter = &g_globals->_party[0];
	return true;
}

bool Temple::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_ESCAPE:
		leave();
		break;
	default:
		break;
	}

	return true;
}

void Temple::draw() {
	Business::draw();
}

} // namespace Businesses
} // namespace Views
} // namespace MM1
} // namespace MM
