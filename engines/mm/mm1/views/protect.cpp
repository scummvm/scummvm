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

#include "mm/mm1/views/protect.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

void Protect::draw() {
	clearScreen();
	writeString(STRING["dialogs.protect.title"]);
	escToGoBack();


}

void Protect::printProtectionFrom() {
	if (_textPos.y == 2)
		writeString(STRING["dialogs.protect.protection"]);
}

void Protect::printProtectionLevel(uint protectIndex) {
	_textPos.x = 24;
	writeChar('+');
	_textPos.x++;
	writeNumber(g_globals->_activeSpells[protectIndex]);
}

bool Protect::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		close();
		return true;
	}

	return false;
}

bool Protect::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_PROTECT) {
		addView();
		return true;
	}

	return false;
}


} // namespace Views
} // namespace MM1
} // namespace MM
