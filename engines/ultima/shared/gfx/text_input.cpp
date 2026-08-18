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

#include "ultima/shared/gfx/text_input.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

#define DELAY_FRAMES 2

TextInput::TextInput(const Common::String &name, UIElement *parent) : UIElement(name, parent) {
}

TextInput::TextInput(const Common::String &name, UIElement *parent, const Common::Point &pos,
	int maxChars, bool isNumeric) : UIElement(name, parent) {
	setBounds(TextRect(pos.x, pos.y, pos.x + maxChars, pos.y + 1));
	_isNumeric = isNumeric;
}


void TextInput::clear() {
	_text.clear();
	redraw();
}

void TextInput::show(const Common::Point &pos, int maxChars, bool isNumeric) {
	setBounds(TextRect(pos.x, pos.y, pos.x + maxChars, pos.y + 1));
	_isNumeric = isNumeric;
	show();
}

void TextInput::show() {
	// Set to visible, and start timer for blinking the cursor
	_visible = true;
	_text.clear();
	delayFrames(DELAY_FRAMES);
	redraw();
}

void TextInput::hide() {
	// Hide the text input
	_visible = false;
	cancelDelay();
	redraw();
}

void TextInput::setTextPos(const Common::Point &pt) {
	setBounds(TextRect(pt.x, pt.y, pt.x, pt.y));
}

void TextInput::draw() {
	auto s = getSurface();

	if (_visible) {
		s.clear();
		s.writeString(_text);

		if (_caretVisible)
			s.writeChar('_');
	}
}

bool TextInput::msgFocus(const FocusMessage &msg) {
	_text.clear();
	return true;
}

bool TextInput::msgUnfocus(const UnfocusMessage &msg) {
	hide();
	return true;
}

bool TextInput::msgKeypress(const KeypressMessage &msg) {
	if (Common::isPrint(msg.ascii)) {
		// Printable character
		if (!_isNumeric || Common::isDigit(msg.keycode)) {
			_text += toupper(msg.ascii);

			auto s = getSurface();
			if (s.getStringWidth(_text) >= _bounds.width())
				_parent->send(GameMessage("INPUT", _text));

			redraw();
		}

		return true;

	} else if (msg.keycode == Common::KEYCODE_BACKSPACE || msg.keycode == Common::KEYCODE_DELETE) {
		if (!_text.empty()) {
			_text.deleteLastChar();
			redraw();
		}
		return true;

	} else if (msg.keycode == Common::KEYCODE_RETURN || msg.keycode == Common::KEYCODE_KP_ENTER) {
		_parent->send(GameMessage("INPUT", _text));
		return true;
	}

	return false;
}

void TextInput::timeout() {
	// Keep the cursor caret blinking
	if (_visible) {
		_caretVisible = !_caretVisible;
		redraw();
		delayFrames(DELAY_FRAMES);
	}
}

} // namespace Gfx
} // namespace Shared
} // namespace Ultima
