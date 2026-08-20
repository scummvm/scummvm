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

#include "ultima/ultima1/gfx/text_input.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

TextInput::TextInput(const Common::String &name, UIElement *parent, TextCursor &cursor) :
		UIElement(name, parent), _cursor(cursor) {
}

void TextInput::setPosition(const Common::Point &pt, int maxWidth) {
	_pos = pt;
	_maxWidth = maxWidth;
	setBounds(TextRect(pt.x, pt.y, pt.x + maxWidth, pt.y));

	_text.clear();
	_visible = true;
	updateCursor();
}

void TextInput::hide() {
	_visible = false;
	_cursor.hide();
	redraw();
}

void TextInput::updateCursor() {
	_cursor.setPosition(Common::Point(_pos.x + _text.size(), _pos.y));
	_cursor.show();
}

void TextInput::draw() {
	if (!_visible)
		return;

	auto s = getSurface();
	s.clear();
	s.setColor(_color);
	s.writeString(_text);
}

bool TextInput::msgUnfocus(const UnfocusMessage &msg) {
	hide();
	return true;
}

bool TextInput::msgKeypress(const KeypressMessage &msg) {
	if (!_visible)
		return false;

	if (Common::isPrint(msg.ascii) && (int)_text.size() < _maxWidth) {
		_text += (char)msg.ascii;
		updateCursor();
		redraw();
		return true;

	} else if (msg.keycode == Common::KEYCODE_BACKSPACE || msg.keycode == Common::KEYCODE_DELETE) {
		if (!_text.empty()) {
			_text.deleteLastChar();
			updateCursor();
			redraw();
		}
		return true;

	} else if (msg.keycode == Common::KEYCODE_RETURN || msg.keycode == Common::KEYCODE_KP_ENTER) {
		_parent->send(GameMessage("INPUT", _text));
		return true;
	}

	return false;
}

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima
