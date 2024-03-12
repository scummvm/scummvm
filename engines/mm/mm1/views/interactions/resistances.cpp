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

#include "mm/mm1/views/interactions/resistances.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

Resistances::Resistances() : TextView("Resistances") {
	_bounds = getLineBounds(20, 24);
}

bool Resistances::msgFocus(const FocusMessage &msg) {
	_charSelected = false;
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_PARTY_MENUS);
	return TextView::msgFocus(msg);
}

bool Resistances::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return TextView::msgUnfocus(msg);
}

void Resistances::draw() {
	clearSurface();
	Sound::sound(SOUND_2);

	if (!_charSelected) {
		writeString(0, 1, STRING["maps.map02.morango"]);

	} else {
		Character &c = *g_globals->_currCharacter;
		writeString(0, 1, STRING["maps.map02.resistances1"]);
		writeString(c._name);
		newLine();
		writeString(0, 1, STRING["maps.map02.resistances2"]);

		writeNumber(6, 1, c._resistances._s._magic);
		writeNumber(15, 1, c._resistances._s._fire);
		writeNumber(26, 1, c._resistances._s._cold);
		writeNumber(36, 1, c._resistances._s._electricity);
		writeNumber(6, 2, c._resistances._s._acid);
		writeNumber(15, 2, c._resistances._s._fear);
		writeNumber(26, 2, c._resistances._s._poison);
		writeNumber(36, 2, c._resistances._s._psychic);
	}
}

bool Resistances::msgKeypress(const KeypressMessage &msg) {
	if (_charSelected)
		close();
	return true;
}

bool Resistances::msgMouseDown(const MouseDownMessage &msg) {
	if (_charSelected)
		close();
	return true;
}

bool Resistances::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();

	} else if (!_charSelected &&
		msg._action >= KEYBIND_VIEW_PARTY1 &&
		msg._action <= KEYBIND_VIEW_PARTY6) {
		uint charIndex = (int)(msg._action - KEYBIND_VIEW_PARTY1);
		if (charIndex < g_globals->_party.size()) {
			_charSelected = true;
			g_globals->_currCharacter = &g_globals->_party[charIndex];
			redraw();
		}	
	}

	return true;
}


} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
