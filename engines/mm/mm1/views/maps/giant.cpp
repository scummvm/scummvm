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

#include "mm/mm1/views/maps/giant.h"
#include "mm/mm1/maps/map30.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

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
	if (isDelayActive()) {
		cancelDelay();
		redraw();
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
	if (isDelayActive()) {
		cancelDelay();
		redraw();
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

	Character &c = g_globals->_party[charIndex];
	g_globals->_currCharacter = &c;

	if ((c._flags[1] & ~CHARFLAG1_WORTHY) == ~CHARFLAG1_WORTHY)
		worthy();
	else
		unworthy();
}

void Giant::timeout() {
	redraw();
}

void Giant::worthy() {
	Character &c = *g_globals->_currCharacter;
	int val = ((c._v6e + 1) / 2) * 256;
	c._exp += val;

	Common::String line = Common::String::format(
		STRING["maps.map30.worthy"].c_str(), '0' + (c._v6e / 5));
	line = Common::String::format("%s%d %s",
		line.c_str(), val, STRING["maps.map30.experience"].c_str());

	if (c._v6e & 0x80) {
		int attrNum = getRandomNumber(7) - 1;
		line += Common::String::format(", +3 %s",
			STRING[Common::String::format("maps.map30.attributes.%d", attrNum)].c_str());

		AttributePair &attrib = c.getAttribute(attrNum);
		if (attrib._base < 43)
			attrib._current = attrib._base = attrib._base + 3;
	}

	c._v6e = 0;

	clearSurface();
	writeString(0, 1, line);
	Sound::sound(SOUND_2);

	delaySeconds(5);
}

void Giant::unworthy() {
	clearSurface();
	writeString(0, 1, STRING["maps.map30.unworthy"]);
	Sound::sound(SOUND_2);

	delaySeconds(5);
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
