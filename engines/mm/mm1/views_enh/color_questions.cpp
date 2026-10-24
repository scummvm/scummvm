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
#include "mm/mm1/data/text.h"
#include "mm/mm1/maps/map17.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define COLOR 510
#define CORRECT_ANSWERS 511

ColorQuestions::ColorQuestions() : ScrollView("ColorQuestions") {
	setBounds(Common::Rect(0, 88, 234, 144));
}

bool ColorQuestions::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);
	_showingResponse = false;

	// Find first non-incapacitated party member.
	_charIndex = -1;
	moveToNextChar();

	// The original view indexed the party unconditionally in draw().
	// Guard an empty/no-eligible party so opening the Hooded Figure quiz
	// can never dereference past the party array.
	if (_charIndex < 0 || _charIndex >= (int)g_globals->_party.size()) {
		close();
		g_globals->_party.checkPartyDead();
		return true;
	}

	return true;
}

void ColorQuestions::draw() {
	if (_charIndex < 0 || _charIndex >= (int)g_globals->_party.size())
		return;

	// Highlight next character to answer question.
	g_globals->_currCharacter = &g_globals->_party[_charIndex];
	g_events->send("GameParty", GameMessage("CHAR_HIGHLIGHT", (int)true));

	ScrollView::draw();
	setReduced(false);

	if (!_showingResponse) {
		writeString(0, 0, "What is your color (1-8)?");

		setReduced(true);
		for (int option = 0; option < 8; ++option) {
			Common::String prefix = Common::String::format("%c) ", '1' + option);
			writeString(20 + 105 * (option % 2), 10 + (option / 2) * 8, prefix, ALIGN_RIGHT);
			writeString(COLOR_NAMES[option]);
		}
	} else {
		const Character &c = g_globals->_party[_charIndex];
		Common::String result = (c.hasBadCondition() ? "Wrong!" : "Correct!");
		writeLine(1, result, ALIGN_MIDDLE);
	}
}

bool ColorQuestions::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	if (_charIndex < 0 || _charIndex >= (int)g_globals->_party.size())
		return true;

	if (!_showingResponse && msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_8) {
		Maps::Map17 *mapP = dynamic_cast<Maps::Map17 *>(g_maps->_currentMap);
		if (!mapP) {
			close();
			return true;
		}

		Maps::Map17 &map = *mapP;
		map[COLOR] = msg.keycode - Common::KEYCODE_1;

		Character &c = g_globals->_party[_charIndex];
		const byte sign = c._flags[4];

		// Gypsy stores the assigned sign in flag byte 4, not byte 2.
		if (!(sign & CHARFLAG4_ASSIGNED) ||
				(sign & CHARFLAG4_SIGN) != map[COLOR]) {
			c._condition = ERADICATED;
		} else {
			map[CORRECT_ANSWERS]++;
			c._flags[4] |= CHARFLAG4_80;
		}

		_showingResponse = true;
		redraw();
		delaySeconds(2);
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

		g_globals->_party.checkPartyDead();

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
