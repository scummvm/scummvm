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

#include "mm/mm1/views/wheel_spin.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

WheelSpin::WheelSpin() : TextView("WheelSpin") {
	_bounds = getLineBounds(17, 24);
}

bool WheelSpin::msgFocus(const FocusMessage &msg) {
	TextView::msgFocus(msg);
	spin();
	return true;
}

void WheelSpin::draw() {
	clearSurface();

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		const Character &c = g_globals->_party[i];
		writeString(0, i, c._name);
		writeString(17, i, _results[i]);
	}
}

bool WheelSpin::msgKeypress(const KeypressMessage &msg) {
	close();
	return true;
}

bool WheelSpin::msgAction(const ActionMessage &msg) {
	close();
	return true;
}

} // namespace Views
} // namespace MM1
} // namespace MM
