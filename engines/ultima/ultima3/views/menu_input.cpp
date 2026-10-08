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

#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

bool isModifierKey(Common::KeyCode key) {
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

char commandKey(const KeypressMessage &msg) {
	char c = (msg.ascii > 0 && msg.ascii < 0x80) ? msg.ascii : 0;
	return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

static bool isEnterKey(Common::KeyCode key) {
	return key == Common::KEYCODE_RETURN || key == Common::KEYCODE_KP_ENTER;
}

static bool isEraseKey(Common::KeyCode key) {
	return key == Common::KEYCODE_BACKSPACE || key == Common::KEYCODE_LEFT;
}

/*------------------------------------------------------------------------*/

void MenuChoice::setup(const char *keys, const char *const *words, int count, int col, int row) {
	_keys = keys;
	_words = words;
	_count = count;
	_col = col;
	_row = row;
	_choice = -1;
}

bool MenuChoice::handleKey(const KeypressMessage &msg) {
	if (isModifierKey(msg.keycode))
		return false;

	if (_choice >= 0) {
		if (isEnterKey(msg.keycode))
			return true;

		if (isEraseKey(msg.keycode))
			_choice = -1;
		else
			g_engine->playErrorBeep();
		return false;
	}

	char c = 0;
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		c = 0x1B;
	} else if (msg.ascii > 0 && msg.ascii < 0x80) {
		c = msg.ascii;
		if (c >= 'a' && c <= 'z')
			c -= 'a' - 'A';
	}

	for (int i = 0; c && i < _count; ++i) {
		if (_keys[i] == c) {
			_choice = i;
			return false;
		}
	}

	g_engine->playErrorBeep();
	return false;
}

void MenuChoice::draw(GfxSurface &s, bool active) const {
	int col = _col;

	if (_choice >= 0) {
		const Common::String word(_words[_choice]);
		s.writeString(Common::Point(_col, _row), word);
		col += word.size();
	}

	if (active) {
		s.setTextPos(Common::Point(col, _row));
		s.writeChar(GLYPH_UP_ARROW);
	}
}

/*------------------------------------------------------------------------*/

void LineInput::setup(uint maxLength, int col, int row) {
	_maxLength = maxLength;
	_col = col;
	_row = row;
	_text.clear();
}

bool LineInput::handleKey(const KeypressMessage &msg) {
	if (isModifierKey(msg.keycode))
		return false;

	if (isEnterKey(msg.keycode))
		return true;

	if (isEraseKey(msg.keycode)) {
		if (_text.empty())
			g_engine->playErrorBeep();
		else
			_text.deleteLastChar();
	} else if (msg.keycode != Common::KEYCODE_DELETE && msg.ascii >= 0x20 && msg.ascii < 0x80 &&
			_text.size() < _maxLength) {
		_text += (char)msg.ascii;
	} else {
		g_engine->playErrorBeep();
	}

	return false;
}

void LineInput::draw(GfxSurface &s, bool active) const {
	s.writeString(Common::Point(_col, _row), _text);

	if (active) {
		s.setTextPos(Common::Point(_col + _text.size(), _row));
		s.writeChar(GLYPH_UP_ARROW);
	}
}

/*------------------------------------------------------------------------*/

bool NumberInput::handleKey(const KeypressMessage &msg) {
	if (!LineInput::handleKey(msg))
		return false;

	// Leading spaces are skipped, and digits are read until the first
	// non-digit. Hex letters are read too, but can't form a decimal number
	uint i = 0;
	while (i < _text.size() && _text[i] == ' ')
		++i;

	uint16 hex = 0;
	int decimal = 0, digits = 0;
	bool isDecimal = true;

	for (; i < _text.size(); ++i, ++digits) {
		char c = _text[i];
		int nibble;

		if (c >= '0' && c <= '9')
			nibble = c - '0';
		else if (c >= 'A' && c <= 'F')
			nibble = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			nibble = c - 'a' + 10;
		else
			break;

		hex = (hex << 4) | nibble;
		if (nibble > 9)
			isDecimal = false;
		else
			decimal = decimal * 10 + nibble;
	}

	if (digits == 0 || !isDecimal) {
		g_engine->playErrorBeep();
		_text.clear();
		return false;
	}

	_bcd = hex & 0xFF;
	_decimal = decimal;
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
