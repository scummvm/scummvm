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

#ifndef ULTIMA1_GFX_TEXT_INPUT_H
#define ULTIMA1_GFX_TEXT_INPUT_H

#include "ultima/shared/engine/events.h"
#include "ultima/ultima1/gfx/text_cursor.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

using namespace Shared::Messages;

/**
 * Simple text entry field. Accepts printable characters up to a maximum
 * width and backspace to delete; Enter sends a GameMessage named "INPUT"
 * to its parent, with the entered text as the string parameter.
 *
 * Rather than drawing its own caret, it drives a shared TextCursor -
 * positioned just after the last typed character - so the same cursor
 * animation is used consistently, whether it's parked at a prompt or
 * following text as it's typed.
 */
class TextInput : public Shared::UIElement {
private:
	Common::String _text;
	Common::Point _pos;
	int _maxWidth = 0;
	bool _visible = false;
	byte _color = 15;
	TextCursor &_cursor;

	/**
	 * Repositions the cursor to just after the last typed character
	 */
	void updateCursor();

public:
	TextInput(const Common::String &name, UIElement *parent, TextCursor &cursor);
	~TextInput() override {}

	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;

	/**
	 * Positions the field and shows it ready for input, clearing any
	 * previously entered text. The field's bounds are sized one
	 * character wider than maxWidth, to leave room for the cursor when
	 * the text is at its maximum length
	 */
	void setPosition(const Common::Point &pt, int maxWidth);

	/**
	 * Hides the field
	 */
	void hide();

	/**
	 * Sets the color the entered text is drawn in
	 */
	void setColor(byte color) {
		_color = color;
	}
};

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima

#endif
