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

#include "mm/mm1/views_enh/text_entry.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

void TextEntry::display(int x, int y, int maxLen,
		bool isNumeric, Abort abortFn, Enter enterFn) {
	_maxLen = maxLen;
	_abortFn = abortFn;
	_enterFn = enterFn;
	_isNumeric = isNumeric;
	_text = "";
	_bounds = Common::Rect(x, y, x + maxLen * 8, y + 9);

	addView(this);
}

void TextEntry::draw() {
	drawText();
	writeChar('_');
}

void TextEntry::drawText() {
	Graphics::ManagedSurface s = getSurface();
	s.clear(0x99);

	writeString(0, 0, _text);
}

bool TextEntry::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_BACKSPACE &&
			!_text.empty()) {
		_text.deleteLastChar();
		redraw();
		return true;

	} else if (msg.ascii >= 32 && msg.ascii <= 127 &&
			_text.size() < _maxLen) {
		if (_isNumeric && (msg.ascii < '0' || msg.ascii > '9'))
			return true;

		_text += msg.ascii;

		// Single character numeric fields, particular spell
		// level/number selection, return immediately
		if (_isNumeric && _maxLen == 1) {
			drawText();
			msgAction(ActionMessage(KEYBIND_SELECT));

		} else {
			redraw();
		}
	}

	return true;
}

bool TextEntry::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		drawText();
		close();
		_abortFn();
		break;

	case KEYBIND_SELECT:
		if (!_text.empty()) {
			drawText();
			close();
			_enterFn(_text);
		}
		break;

	default:
		break;
	}

	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
