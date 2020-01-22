/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/shared/gfx/text_input.h"
#include "ultima/shared/gfx/text_cursor.h"
#include "ultima/shared/early/game_base.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

BEGIN_MESSAGE_MAP(TextInput, Popup)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

void TextInput::show(const Point &pt, bool isNumeric, size_t maxCharacters, byte color, TreeItem *respondTo) {
	Popup::show(respondTo);
	_isNumeric = isNumeric;
	_maxCharacters = maxCharacters;
	_color = color;
	_bounds = Rect(pt.x, pt.y, pt.x + 8 * (maxCharacters + 1), pt.y + 8);
	_text = "";

	_game->_textCursor->setPosition(Point(_bounds.left, _bounds.top));
	_game->_textCursor->setVisible(true);
}

void TextInput::draw() {
	Popup::draw();

	VisualSurface s = getSurface();

	// Ensure the cursor is at the right position
	_game->_textCursor->setPosition(Point(_bounds.left + _text.size() * 8, _bounds.top));

	// Display the text
	Common::String text = _text;
	while (text.size() < _maxCharacters)
		text += ' ';
	s.writeString(text, TextPoint(0, 0), _color);
}

bool TextInput::KeypressMsg(CKeypressMsg &msg) {
	uint16 c = msg._keyState.ascii;
	//TreeItem *respondTo = _respondTo;

	if (c >= ' ' && c <= 0x7f) {
		// Printable character
		if (_text.size() < _maxCharacters && (!_isNumeric || (c >= '0' && c <= '9'))) {
			_text += msg._keyState.ascii;
			setDirty();
		}
	} else if (msg._keyState.keycode == Common::KEYCODE_BACKSPACE || msg._keyState.keycode == Common::KEYCODE_LEFT) {
		if (!_text.empty()) {
			_text.deleteLastChar();
			setDirty();
		}
	} else if (msg._keyState.keycode == Common::KEYCODE_RETURN || msg._keyState.keycode == Common::KEYCODE_KP_ENTER) {
		_game->_textCursor->setVisible(false);
		hide();

		CTextInputMsg inputMsg(_text, false);
		inputMsg.execute(_respondTo);
	} else if (msg._keyState.keycode == Common::KEYCODE_ESCAPE) {
		_game->_textCursor->setVisible(false);
		hide();

		CTextInputMsg inputMsg("", true);
		inputMsg.execute(_respondTo);
	}

	return true;
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
