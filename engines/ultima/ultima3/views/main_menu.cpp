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

#include "common/keyboard.h"
#include "ultima/ultima3/views/main_menu.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int GLYPH_UP_ARROW = 0x1E;
constexpr int BORDER_TOP = 10;
constexpr int BORDER_BOTTOM = 23;
constexpr int BORDER_RIGHT = 39;
constexpr int OPTION_ROW = 15;
constexpr int OPTION_COL = 16;
constexpr int CHOICE_COL = 23;

// Letter keys for each option, with Escape (the last) acting as Return
static const char OPTION_KEYS[] = { 'R', 'O', 'J' };
static const char *const OPTION_WORDS[] = { "Return", "Organize", "Journey", "Return" };
constexpr int OPTION_ESCAPE = 3;

static bool isModifierKey(Common::KeyCode key) {
	switch (key) {
	case Common::KEYCODE_LSHIFT:
	case Common::KEYCODE_RSHIFT:
	case Common::KEYCODE_LCTRL:
	case Common::KEYCODE_RCTRL:
	case Common::KEYCODE_LALT:
	case Common::KEYCODE_RALT:
	case Common::KEYCODE_LMETA:
	case Common::KEYCODE_RMETA:
	case Common::KEYCODE_CAPSLOCK:
	case Common::KEYCODE_NUMLOCK:
	case Common::KEYCODE_SCROLLOCK:
		return true;
	default:
		return false;
	}
}

static int findOption(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE)
		return OPTION_ESCAPE;

	char c = msg.ascii;
	if (c >= 'a' && c <= 'z')
		c -= 'a' - 'A';

	for (int i = 0; i < ARRAYSIZE(OPTION_KEYS); ++i) {
		if (OPTION_KEYS[i] == c)
			return i;
	}

	return -1;
}

bool MainMenu::msgFocus(const FocusMessage &msg) {
	_state = CHOOSING;
	return View::msgFocus(msg);
}

void MainMenu::drawBorder(GfxSurface &s) {
	// Alternating black and magenta pixels, filling whole 8x8 cells
	auto cell = [&](int col, int row) {
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < 8; ++x)
				s.setPixel(col * 8 + x, row * 8 + y, (x & 1) ? 2 : 0);
	};

	s.fillRect(Common::Rect(0, 24 * 8, 320, 25 * 8), 0);

	for (int col = 0; col <= BORDER_RIGHT; ++col)
		cell(col, BORDER_BOTTOM);
	for (int col = 1; col < BORDER_RIGHT; ++col)
		cell(col, BORDER_TOP);
	for (int row = BORDER_TOP; row <= BORDER_BOTTOM; ++row) {
		cell(0, row);
		cell(BORDER_RIGHT, row);
	}
}

void MainMenu::draw() {
	auto s = getSurface();

	s.fillRect(Common::Rect(8, 11 * 8, 39 * 8, 23 * 8), 0);
	drawBorder(s);

	s.writeString(Common::Point(3, 21), "(C)-1983 By James R. Van Artsdalen");
	s.writeString(Common::Point(13, 22), "and Lord British");
	s.writeString(Common::Point(7, 11), "From the depths of hell...");
	s.writeString(Common::Point(7, 12), "...he comes for VENGEANCE!");
	s.writeString(Common::Point(OPTION_COL, OPTION_ROW), "Option: ");
	s.writeString(Common::Point(11, 17), "Return to the View");
	s.writeString(Common::Point(12, 18), "Organize a Party");
	s.writeString(Common::Point(13, 19), "Journey Onward");

	s.setTextPos(Common::Point(CHOICE_COL, OPTION_ROW));
	if (_state == CONFIRMING) {
		const char *word = OPTION_WORDS[_choice];
		s.writeString(word);
	}
	s.writeChar(GLYPH_UP_ARROW);
}

bool MainMenu::msgKeypress(const KeypressMessage &msg) {
	if (isModifierKey(msg.keycode))
		return true;

	if (_state == CONFIRMING) {
		switch (msg.keycode) {
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			// The confirmed option is wired up in later stages
			break;
		case Common::KEYCODE_BACKSPACE:
		case Common::KEYCODE_LEFT:
			_state = CHOOSING;
			redraw();
			break;
		default:
			g_engine->playErrorBeep();
			break;
		}
	} else {
		int option = findOption(msg);
		if (option >= 0) {
			_choice = option;
			_state = CONFIRMING;
			redraw();
		} else {
			g_engine->playErrorBeep();
		}
	}

	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
