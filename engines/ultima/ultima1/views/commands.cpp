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

#include "ultima/ultima1/views/commands.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

Commands::Commands(UIElement *parent) : Shared::UIElement("Commands", parent), _textCursor("CommandsCursor", this) {
}

bool Commands::msgFocus(const FocusMessage &msg) {
	return true;
}

void Commands::draw() {
	// Get a surface. We'll maintain a persistent copy to maintain
	// the text pos across multiple calls
	_surface = getSurface();
	_surface.clear();
	_surface.setTextPos(Common::Point(0, 4));
	_surface.setScrollable(true);

	prompt();
}

bool Commands::msgGame(const GameMessage &msg) {
	if (msg._name == "TEXT") {
		writeString(msg._stringValue);
		return true;
	} else if (msg._name == "PROMPT") {
		prompt();
		return true;
	}

	return false;
}

bool Commands::msgAction(const ActionMessage &msg) {
	g_engine->_logic->action(msg._action);
	return true;
}

bool Commands::msgKeypress(const KeypressMessage &msg) {
	g_engine->_logic->keypress(msg.keycode);
	return true;
}

void Commands::writeString(const Common::String &msg) {
	_textCursor.hide();
	_surface.writeString(msg);
}

void Commands::prompt() {
	writeString("\x10");
	_textCursor.setPosition(Common::Point(1, 24));
	_textCursor.show();
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
