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

#include "mm/mm1/views_enh/wheel_spin.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

WheelSpin::WheelSpin() : ScrollView("WheelSpin") {
	setBounds(Common::Rect(0, 90, 234, 200));
}

bool WheelSpin::msgFocus(const FocusMessage &msg) {
	TextView::msgFocus(msg);
	spin();
	return true;
}

void WheelSpin::draw() {
	ScrollView::draw();

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		const Character &c = g_globals->_party[i];
		writeLine(i, c._name, ALIGN_LEFT, 0);
		writeLine(i, _results[i], ALIGN_LEFT, 100);
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

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
