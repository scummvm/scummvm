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

#include "ultima/ultima1/views/interactions/direction.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

bool Direction::msgFocus(const FocusMessage &msg) {
	showCursor();
	return true;
}

bool Direction::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_UP:
	case KEYBIND_ATTACK_UP:
		_G(logic)->action(KEYBIND_UP);
		break;
	case KEYBIND_DOWN:
	case KEYBIND_ATTACK_DOWN:
		_G(logic)->action(KEYBIND_DOWN);
		break;
	case KEYBIND_LEFT:
	case KEYBIND_ATTACK_LEFT:
		_G(logic)->action(KEYBIND_LEFT);
		break;
	case KEYBIND_RIGHT:
	case KEYBIND_ATTACK_RIGHT:
		_G(logic)->action(KEYBIND_RIGHT);
		break;
	default:
		_G(logic)->action(KEYBIND_PASS);
		break;
	}

	return true;
}

bool Direction::msgKeypress(const KeypressMessage &msg) {
	close();
	_G(logic)->action(KEYBIND_PASS);
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
