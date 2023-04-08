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

#include "mm/mm1/views_enh/color_questions.h"
#include "mm/mm1/maps/map17.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define COLOR 510
#define CORRECT_ANSWERS 511

ColorQuestions::ColorQuestions() : ScrollView("ColorQuestions") {
	setBounds(Common::Rect(0, 144, 234, 200));
}

bool ColorQuestions::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);
	_showingResponse = false;

	// Find first non-incapacitated party member
	_charIndex = -1;
	moveToNextChar();

	return true;
}

void ColorQuestions::draw() {
	clearSurface();

	writeString(0, 0, STRING["maps.map17.color"]);
	writeString(0, 2, STRING["maps.map17.options"]);

	if (_showingResponse) {
		const Character &c = g_globals->_party[_charIndex];

		Common::String result = STRING[c.hasBadCondition() ?
			"maps.map17.wrong" : "maps.map17.correct"];
		writeString(16, 6, result);
	}

}

bool ColorQuestions::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	if (!_showingResponse && msg.keycode >= Common::KEYCODE_1 && msg.keycode <= Common::KEYCODE_9) {
		Maps::Map17 &map = *static_cast<Maps::Map17 *>(g_maps->_currentMap);
		map[COLOR] = msg.ascii - '1';

		Character &c = *g_globals->_currCharacter;
		int color = c._flags[2] & 0xf;

		// If a color hasn't been designated yet from talking to Gypsy,
		// or it has but the wrong color is selected, eradicate them
		if (!color || (color & 7) != map[COLOR]) {
			c._condition = ERADICATED;
		} else {
			map[CORRECT_ANSWERS]++;
			c._flags[4] |= CHARFLAG4_80;
		}

		// Show the response
		_showingResponse = true;
		redraw();

		return true;
	}

	return false;
}

bool ColorQuestions::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	return false;
}

void ColorQuestions::timeout() {
	// Move to next non-incapacitated character
	_showingResponse = false;
	moveToNextChar();

	if (_charIndex >= (int)g_globals->_party.size()) {
		// All of party questioned
		close();
		g_maps->_mapPos.y = 2;
		g_maps->_currentMap->updateGame();

	} else {
		// Prompt response for next party member
		redraw();
	}
}

void ColorQuestions::moveToNextChar() {
	do {
		++_charIndex;
	} while (_charIndex < (int)g_globals->_party.size() &&
		g_globals->_party[_charIndex].hasBadCondition());
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
