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

#include "ultima/ultima3/views/interactions/log_input.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

void LogMenu::setup(const char *keys, const char *const *words, int count) {
	_menu.setup(keys, words, count, 0, 0);
	_echo.clear();
}

bool LogMenu::handleKey(const KeypressMessage &msg) {
	bool hadChoice = _menu.hasChoice();
	bool confirmed = _menu.handleKey(msg);

	if (!hadChoice && _menu.hasChoice()) {
		_echo = _menu.word();
		_G(messages).print(_echo.c_str());
	} else if (hadChoice && !_menu.hasChoice()) {
		_G(messages).backspace(_echo.size());
		_echo.clear();
	}

	return confirmed;
}

/*------------------------------------------------------------------------*/

LogYesNo::LogYesNo() {
	static const char *const WORDS[4] = { "Yes", "No", "No", "No" };
	_menu.setup("YNQ\033", WORDS, 4);
}

/*------------------------------------------------------------------------*/

void LogNumber::setup(int maxDigits, bool rejectLetters) {
	_maxDigits = maxDigits;
	_rejectLetters = rejectLetters;
	_text.clear();
	_bcd = 0;
	_hasLetters = false;
}

static int hexDigit(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}

bool LogNumber::evaluate() {
	uint pos = 0;
	while (pos < _text.size() && _text[pos] == ' ')
		++pos;

	// There has to be a digit to start with, and the number ends where they stop
	if (pos == _text.size() || hexDigit(_text[pos]) < 0)
		return false;

	_bcd = 0;
	_hasLetters = false;
	for (; pos < _text.size(); ++pos) {
		int digit = hexDigit(_text[pos]);
		if (digit < 0)
			break;

		_bcd = (_bcd << 4) | digit;
		if (digit > 9)
			_hasLetters = true;
	}

	return !(_rejectLetters && _hasLetters);
}

bool LogNumber::handleKey(const KeypressMessage &msg) {
	if (isModifierKey(msg.keycode))
		return false;

	if (msg.keycode == Common::KEYCODE_RETURN || msg.keycode == Common::KEYCODE_KP_ENTER) {
		if (evaluate())
			return true;

		// Not a number, so it's wiped and tried again
		_G(messages).backspace(_text.size());
		_text.clear();
		g_engine->playSoundEffect(0xFE);
		return false;
	}

	if (msg.keycode == Common::KEYCODE_BACKSPACE || msg.keycode == Common::KEYCODE_LEFT ||
			msg.keycode == Common::KEYCODE_DELETE) {
		if (_text.empty()) {
			g_engine->playSoundEffect(0xFE);
		} else {
			_text.deleteLastChar();
			_G(messages).backspace(1);
		}

		return false;
	}

	if (msg.ascii >= 0x20 && msg.ascii < 0x80 && (int)_text.size() < _maxDigits) {
		_text += (char)msg.ascii;
		_G(messages).putChar((char)msg.ascii);
	} else {
		g_engine->playSoundEffect(0xFE);
	}

	return false;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
