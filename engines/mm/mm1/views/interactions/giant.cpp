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

#include "mm/mm1/views/interactions/giant.h"
#include "mm/mm1/maps/map30.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

Giant::Giant() : TextView("Giant") {
	_bounds = getLineBounds(20, 24);
}

bool Giant::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_PARTY_MENUS);
	return TextView::msgFocus(msg);
}

bool Giant::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return TextView::msgUnfocus(msg);
}

void Giant::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map30.giant"]);
}

bool Giant::msgKeypress(const KeypressMessage &msg) {
	if (endDelay()) {
		draw();

	} else if (msg.keycode < Common::KEYCODE_1 ||
		msg.keycode > Common::KEYCODE_6) {
		close();

	} else {
		uint charIndex = msg.keycode - Common::KEYCODE_1;
		charSelected(charIndex);
	}

	return true;
}

bool Giant::msgAction(const ActionMessage &msg) {
	if (endDelay()) {
		// Nothing further

	} else if (msg._action >= KEYBIND_VIEW_PARTY1 &&
			msg._action <= KEYBIND_VIEW_PARTY6) {
		uint idx = msg._action - KEYBIND_VIEW_PARTY1;
		charSelected(idx);
	}

	return true;
}

void Giant::charSelected(uint charIndex) {
	if (charIndex >= g_globals->_party.size())
		return;

	MM1::Maps::Map30 &map = *static_cast<MM1::Maps::Map30 *>(g_maps->_currentMap);
	g_globals->_currCharacter = &g_globals->_party[charIndex];
	Common::String line = map.worthiness();

	clearSurface();
	writeString(0, 1, line);
	Sound::sound(SOUND_2);

	delaySeconds(5);
}

void Giant::timeout() {
	redraw();
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
