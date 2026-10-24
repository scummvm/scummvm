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
#include "mm/mm1/data/text.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

void Protect::draw() {
	clearSurface();
	writeString("Protect:  Spells currently active");
	escToGoBack();

	// Protection spells
	_textPos.x = 0;
	_textPos.y = 2;
	for (int i = 0; i < 7; ++i) {
		if (g_globals->_activeSpells._arr[i]) {
			printProtectionFrom();
			writeString(PROTECTION_NAMES[i]);
			printProtectionLevel(i);
		}
	}

	// Light spell
	if (g_globals->_activeSpells._arr[7]) {
		writeString("Light (");
		writeNumber(g_globals->_activeSpells._arr[7]);
		writeChar(')');
		newLine();
	}

	// The rest
	for (int i = 8; i < 18; ++i) {
		if (i == 13)
			newLine();

		if (g_globals->_activeSpells._arr[i]) {
			writeString(PROTECTION_NAMES[i]);
			newLine();
		}
	}

	--_textPos.y;
	_textPos.x = 13;

	if (g_globals->_activeSpells._arr[17]) {
		writeNumber(g_globals->_activeSpells._arr[17]);
		_textPos.x++;
		writeString("to attacks");
	}
}

void Protect::printProtectionFrom() {
	writeString("Protection from");
	_textPos.x = 17;
}

void Protect::printProtectionLevel(uint protectIndex) {
	_textPos.x = 24;
	writeChar('+');
	_textPos.x++;
	writeNumber(g_globals->_activeSpells._arr[protectIndex]);
	writeChar('%');
	newLine();
}

bool Protect::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_PROTECT:
		addView();
		return true;
	case KEYBIND_ESCAPE:
		close();
		return true;
	default:
		break;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
