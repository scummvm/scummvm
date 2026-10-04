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

#ifndef ULTIMA3_VIEWS_MENU_INPUT_H
#define ULTIMA3_VIEWS_MENU_INPUT_H

#include "common/keyboard.h"
#include "common/str.h"
#include "graphics/views/view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

using namespace Graphics::Views;

constexpr int GLYPH_UP_ARROW = 0x1E;

/**
 * Returns true for keys that are only a modifier being pressed
 */
bool isModifierKey(Common::KeyCode key);

/**
 * A menu prompt. Pressing one of the option keys only echoes the matching
 * word; it then has to be confirmed with Enter, or taken back with
 * Backspace or Left. Anything else is rejected with a beep.
 */
class MenuChoice {
private:
	const char *_keys = nullptr;
	const char *const *_words = nullptr;
	int _count = 0;
	int _col = 0, _row = 0;
	int _choice = -1;

public:
	/**
	 * Sets up the prompt. The word at each index is echoed for the key at the
	 * same index; the Escape key is represented in the keys by 0x1B
	 */
	void setup(const char *keys, const char *const *words, int count, int col, int row);

	void reset() {
		_choice = -1;
	}

	/**
	 * Returns true if a choice has been echoed and is awaiting confirmation
	 */
	bool hasChoice() const {
		return _choice >= 0;
	}

	/**
	 * Returns the key of the choice made
	 */
	char key() const {
		return _keys[_choice];
	}

	/**
	 * Handles a keypress, returning true once a choice has been confirmed
	 */
	bool handleKey(const KeypressMessage &msg);

	/**
	 * Draws the echoed word, and the cursor if the prompt is still active
	 */
	void draw(GfxSurface &s, bool active) const;
};

/**
 * A line of text being typed at a cell position, with the cursor following it
 */
class LineInput {
protected:
	Common::String _text;
	uint _maxLength = 0;
	int _col = 0, _row = 0;

public:
	virtual ~LineInput() {}

	void setup(uint maxLength, int col, int row);

	void reset() {
		_text.clear();
	}

	const Common::String &text() const {
		return _text;
	}

	/**
	 * Handles a keypress, returning true once the entry has been accepted
	 */
	virtual bool handleKey(const KeypressMessage &msg);

	void draw(GfxSurface &s, bool active) const;
};

/**
 * A two character entry that has to be a valid decimal number. It's
 * available as the decimal value, and as the BCD value the game stores
 */
class NumberInput : public LineInput {
private:
	byte _bcd = 0;
	int _decimal = 0;

public:
	void setup(int col, int row) {
		LineInput::setup(2, col, row);
	}

	bool handleKey(const KeypressMessage &msg) override;

	byte bcd() const {
		return _bcd;
	}

	int decimal() const {
		return _decimal;
	}
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
