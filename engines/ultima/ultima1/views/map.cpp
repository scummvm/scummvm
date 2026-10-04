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

#include "ultima/ultima1/views/map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

constexpr int PASS_TIME = 4 * 60 * 1000;

Map::Map(const Common::String &name) : Dialog(name) {
}

bool Map::msgFocus(const FocusMessage &msg) {
	_passStartTime = g_system->getMillis();
	return Dialog::msgFocus(msg);
}

bool Map::msgAction(const ActionMessage &msg) {
	_passStartTime = g_system->getMillis();
	g_engine->_logic->action(msg._action);
	return true;
}

bool Map::msgKeypress(const KeypressMessage &msg) {
	_passStartTime = g_system->getMillis();
	g_engine->_logic->keypress(msg.keycode);
	return true;
}

bool Map::tick() {
	uint32 time = g_system->getMillis();

	if (time >= (_passStartTime + PASS_TIME)) {
		_passStartTime = time;
		msgAction(ActionMessage(KEYBIND_PASS));
	}

	if (_G(logic))
		_G(logic)->tick();

	return Dialog::tick();
}


} // namespace Views
} // namespace Ultima1
} // namespace Ultima
